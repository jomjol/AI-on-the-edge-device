# SPDX-FileCopyrightText: 2022-2023 Espressif Systems (Shanghai) CO LTD
# SPDX-License-Identifier: Unlicense OR CC0-1.0
import re
import select
import socket
import struct
import subprocess
import time
from threading import Event, Thread

try:
    import dpkt
    import dpkt.dns
except ImportError:
    pass


def get_dns_query_for_esp(esp_host):
    dns = dpkt.dns.DNS(
        b'\x00\x00\x01\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\x00\x01\x00\x01'
    )
    dns.qd[0].name = esp_host + u'.local'
    print('Created query for esp host: {} '.format(dns.__repr__()))
    return dns.pack()


def get_dns_answer_to_mdns(tester_host):
    dns = dpkt.dns.DNS(
        b'\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00'
    )
    dns.op = dpkt.dns.DNS_QR | dpkt.dns.DNS_AA
    dns.rcode = dpkt.dns.DNS_RCODE_NOERR
    arr = dpkt.dns.DNS.RR()
    arr.cls = dpkt.dns.DNS_IN
    arr.type = dpkt.dns.DNS_A
    arr.name = tester_host
    arr.ip = socket.inet_aton('127.0.0.1')
    dns.an.append(arr)
    print('Created answer to mdns query: {} '.format(dns.__repr__()))
    return dns.pack()


def get_dns_answer_to_mdns_lwip(tester_host, id):
    dns = dpkt.dns.DNS(
        b'\x5e\x39\x84\x00\x00\x01\x00\x01\x00\x00\x00\x00\x0a\x64\x61\x76\x69\x64'
        b'\x2d\x63\x6f\x6d\x70\x05\x6c\x6f\x63\x61\x6c\x00\x00\x01\x00\x01\xc0\x0c'
        b'\x00\x01\x00\x01\x00\x00\x00\x0a\x00\x04\xc0\xa8\x0a\x6c')
    dns.qd[0].name = tester_host
    dns.an[0].name = tester_host
    dns.an[0].ip = socket.inet_aton('127.0.0.1')
    dns.an[0].rdata = socket.inet_aton('127.0.0.1')
    dns.id = id
    print('Created answer to mdns (lwip) query: {} '.format(dns.__repr__()))
    return dns.pack()


def mdns_server(esp_host, events):
    UDP_IP = '0.0.0.0'
    UDP_PORT = 5353
    MCAST_GRP = '224.0.0.251'
    TESTER_NAME = u'tinytester.local'
    TESTER_NAME_LWIP = u'tinytester-lwip.local'
    QUERY_TIMEOUT = 0.2
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEPORT, 1)
    sock.setblocking(False)
    sock.bind((UDP_IP, UDP_PORT))
    mreq = struct.pack('4sl', socket.inet_aton(MCAST_GRP), socket.INADDR_ANY)
    sock.setsockopt(socket.IPPROTO_IP, socket.IP_ADD_MEMBERSHIP, mreq)
    last_query_timepoint = time.time()
    while not events['stop'].is_set():
        try:
            current_time = time.time()
            if current_time - last_query_timepoint > QUERY_TIMEOUT:
                last_query_timepoint = current_time
                if not events['esp_answered'].is_set():
                    sock.sendto(get_dns_query_for_esp(esp_host),
                                (MCAST_GRP, UDP_PORT))
                if not events['esp_delegated_answered'].is_set():
                    sock.sendto(get_dns_query_for_esp(esp_host + '-delegated'),
                                (MCAST_GRP, UDP_PORT))
            timeout = max(
                0, QUERY_TIMEOUT - (current_time - last_query_timepoint))
            read_socks, _, _ = select.select([sock], [], [], timeout)
            if not read_socks:
                continue
            data, addr = sock.recvfrom(1024)
            dns = dpkt.dns.DNS(data)
            if len(dns.qd) > 0:
                for dns_query in dns.qd:
                    if dns_query.type == dpkt.dns.DNS_A:
                        if dns_query.name == TESTER_NAME:
                            print('Received query: {} '.format(dns.__repr__()))
                            sock.sendto(get_dns_answer_to_mdns(TESTER_NAME),
                                        (MCAST_GRP, UDP_PORT))
                        elif dns_query.name == TESTER_NAME_LWIP:
                            print('Received query: {} '.format(dns.__repr__()))
                            sock.sendto(
                                get_dns_answer_to_mdns_lwip(TESTER_NAME_LWIP, dns.id),
                                addr)
            if len(dns.an) > 0:
                for dns_answer in dns.an:
                    if dns_answer.type == dpkt.dns.DNS_A:
                        print('Received answer from {}'.format(dns_answer.name))
                        if dns_answer.name == esp_host + u'.local':
                            print('Received answer to esp32-mdns query: {}'.format(
                                dns.__repr__()))
                            events['esp_answered'].set()
                        if dns_answer.name == esp_host + u'-delegated.local':
                            print('Received answer to esp32-mdns-delegate query: {}'.format(
                                dns.__repr__()))
                            events['esp_delegated_answered'].set()
        except socket.timeout:
            break
        except dpkt.UnpackError:
            continue


def test_examples_protocol_mdns(dut):
    """
    steps: |
      1. obtain IP address + init mdns example
      2. get the dut host name (and IP address)
      3. check the mdns name is accessible
      4. check DUT output if mdns advertized host is resolved
      5. check if DUT responds to dig
      6. check the DUT is searchable via reverse IP lookup
    """

    specific_host = dut.expect(r'mdns hostname set to: \[(.*?)\]')[1].decode()

    mdns_server_events = {
        'stop': Event(),
        'esp_answered': Event(),
        'esp_delegated_answered': Event()
    }
    mdns_responder = Thread(target=mdns_server,
                            args=(str(specific_host), mdns_server_events))
    ip_addresses = []
    if dut.app.sdkconfig.get('LWIP_IPV4') is True:
        ipv4 = dut.expect(r'IPv4 address: (\d+\.\d+\.\d+\.\d+)[^\d]',
                          timeout=30)[1].decode()
        ip_addresses.append(ipv4)
    if dut.app.sdkconfig.get('LWIP_IPV6') is True:
        ipv6_r = r':'.join((r'[0-9a-fA-F]{4}', ) * 8)
        ipv6 = dut.expect(ipv6_r, timeout=30)[0].decode()
        ip_addresses.append(ipv6)
    print('Connected with IP addresses: {}'.format(','.join(ip_addresses)))
    try:
        # TODO: Add test for example disabling IPV4
        mdns_responder.start()
        if dut.app.sdkconfig.get('LWIP_IPV4') is True:
            # 3. check the mdns name is accessible.
            if not mdns_server_events['esp_answered'].wait(timeout=30):
                raise ValueError(
                    'Test has failed: did not receive mdns answer within timeout')
            if not mdns_server_events['esp_delegated_answered'].wait(timeout=30):
                raise ValueError(
                    'Test has failed: did not receive mdns answer for delegated host within timeout'
                )
            # 4. check DUT output if mdns advertized host is resolved
            dut.expect(
                re.compile(
                    b'mdns-test: Query A: tinytester.local resolved to: 127.0.0.1')
            )
            dut.expect(
                re.compile(
                    b'mdns-test: gethostbyname: tinytester-lwip.local resolved to: 127.0.0.1'
                ))
            dut.expect(
                re.compile(
                    b'mdns-test: getaddrinfo: tinytester-lwip.local resolved to: 127.0.0.1'
                ))
            # 5. check the DUT answers to `dig` command
            dig_output = subprocess.check_output([
                'dig', '+short', '-p', '5353', '@224.0.0.251',
                '{}.local'.format(specific_host)
            ])
            print('Resolving {} using "dig" succeeded with:\n{}'.format(
                specific_host, dig_output))
            if not ipv4.encode('utf-8') in dig_output:
                raise ValueError(
                    'Test has failed: Incorrectly resolved DUT hostname using dig'
                    "Output should've contained DUT's IP address:{}".format(ipv4))
            # 6. check the DUT reverse lookup
            if dut.app.sdkconfig.get('MDNS_RESPOND_REVERSE_QUERIES') is True:
                for ip_address in ip_addresses:
                    dig_output = subprocess.check_output([
                        'dig', '+short', '-p', '5353', '@224.0.0.251', '-x',
                        '{}'.format(ip_address)
                    ])
                    print('Reverse lookup for {} using "dig" succeeded with:\n{}'.
                          format(ip_address, dig_output))
                    if specific_host not in dig_output.decode():
                        raise ValueError(
                            'Test has failed: Incorrectly resolved DUT IP address using dig'
                            "Output should've contained DUT's name:{}".format(
                                specific_host))

    finally:
        mdns_server_events['stop'].set()
        mdns_responder.join()

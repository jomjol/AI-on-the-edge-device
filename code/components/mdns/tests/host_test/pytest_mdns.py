# SPDX-FileCopyrightText: 2024 Espressif Systems (Shanghai) CO LTD
# SPDX-License-Identifier: Unlicense OR CC0-1.0
import logging

import pexpect
import pytest
from dnsfixture import DnsPythonWrapper

# Configure logging
logging.basicConfig(level=logging.INFO)
logger = logging.getLogger(__name__)
ipv6_enabled = False


class MdnsConsole:
    def __init__(self, command):
        self.process = pexpect.spawn(command, encoding='utf-8')
        self.process.logfile = open('mdns_interaction.log', 'w')  # Log all interactions
        self.process.expect('mdns> ', timeout=10)

    def send_input(self, input_data):
        logger.info(f'Sending to stdin: {input_data}')
        self.process.sendline(input_data)

    def get_output(self, expected_data):
        logger.info(f'Expecting: {expected_data}')
        self.process.expect(expected_data, timeout=10)
        output = self.process.before.strip()
        logger.info(f'Received from stdout: {output}')
        return output

    def terminate(self):
        self.send_input('exit')
        self.get_output('Exit')
        self.process.wait()
        self.process.close()
        assert self.process.exitstatus == 0


@pytest.fixture(scope='module')
def mdns_console():
    app = MdnsConsole('./build_linux_console/mdns_host.elf')
    yield app
    app.terminate()


@pytest.fixture(scope='module')
def dig_app():
    return DnsPythonWrapper()


def test_mdns_init(mdns_console, dig_app):
    mdns_console.send_input('mdns_init -h hostname')
    mdns_console.get_output('MDNS: Hostname: hostname')
    dig_app.check_record('hostname.local', query_type='A', expected=True)
    if ipv6_enabled:
        dig_app.check_record('hostname.local', query_type='AAAA', expected=True)


def test_add_service(mdns_console, dig_app):
    mdns_console.send_input('mdns_service_add _http _tcp 80 -i test_service')
    mdns_console.get_output('MDNS: Service Instance: test_service')
    mdns_console.send_input('mdns_service_lookup _http _tcp')
    mdns_console.get_output('PTR : test_service')
    dig_app.check_record('_http._tcp.local', query_type='PTR', expected=True)


def test_remove_service(mdns_console, dig_app):
    mdns_console.send_input('mdns_service_remove _http _tcp')
    mdns_console.send_input('mdns_service_lookup _http _tcp')
    mdns_console.get_output('No results found!')
    dig_app.check_record('_http._tcp.local', query_type='PTR', expected=False)


def test_delegate_host(mdns_console, dig_app):
    mdns_console.send_input('mdns_delegate_host delegated 1.2.3.4')
    dig_app.check_record('delegated.local', query_type='A', expected=True)


def test_undelegate_host(mdns_console, dig_app):
    mdns_console.send_input('mdns_undelegate_host delegated')
    dig_app.check_record('delegated.local', query_type='A', expected=False)


def test_add_delegated_service(mdns_console, dig_app):
    mdns_console.send_input('mdns_delegate_host delegated 1.2.3.4')
    dig_app.check_record('delegated.local', query_type='A', expected=True)
    mdns_console.send_input('mdns_service_add _test _tcp 80 -i local')
    mdns_console.get_output('MDNS: Service Instance: local')
    mdns_console.send_input('mdns_service_add _test2 _tcp 80 -i extern -h delegated')
    mdns_console.get_output('MDNS: Service Instance: extern')
    mdns_console.send_input('mdns_service_lookup _test _tcp')
    mdns_console.get_output('PTR : local')
    mdns_console.send_input('mdns_service_lookup _test2 _tcp -d')
    mdns_console.get_output('PTR : extern')
    dig_app.check_record('_test2._tcp.local', query_type='PTR', expected=True)
    dig_app.check_record('extern._test2._tcp.local', query_type='SRV', expected=True)


def test_remove_delegated_service(mdns_console, dig_app):
    mdns_console.send_input('mdns_service_remove _test2 _tcp -h delegated')
    mdns_console.send_input('mdns_service_lookup _test2 _tcp -d')
    mdns_console.get_output('No results found!')
    dig_app.check_record('_test2._tcp.local', query_type='PTR', expected=False)
    # add the delegated service again, would be used in the TXT test
    mdns_console.send_input('mdns_service_add _test2 _tcp 80 -i extern -h delegated')
    mdns_console.get_output('MDNS: Service Instance: extern')


def check_txt_for_service(instance, service, proto, mdns_console, dig_app, host=None, with_inst=False):
    for_host_arg = f'-h {host}' if host is not None else ''
    for_inst_arg = f'-i {instance}' if with_inst else ''
    mdns_console.send_input(f'mdns_service_txt_set {service} {proto} {for_host_arg} {for_inst_arg} key1 value1')
    dig_app.check_record(f'{instance}.{service}.{proto}.local', query_type='SRV', expected=True)
    dig_app.check_record(f'{service}.{proto}.local', query_type='TXT', expected=True, expect='key1=value1')
    mdns_console.send_input(f'mdns_service_txt_set {service} {proto} {for_host_arg} {for_inst_arg} key2 value2')
    dig_app.check_record(f'{service}.{proto}.local', query_type='TXT', expected=True, expect='key2=value2')
    mdns_console.send_input(f'mdns_service_txt_remove {service} {proto} {for_host_arg} {for_inst_arg} key2')
    dig_app.check_record(f'{service}.{proto}.local', query_type='TXT', expected=False, expect='key2=value2')
    dig_app.check_record(f'{service}.{proto}.local', query_type='TXT', expected=True, expect='key1=value1')
    mdns_console.send_input(f'mdns_service_txt_replace {service} {proto} {for_host_arg}  {for_inst_arg} key3=value3 key4=value4')
    dig_app.check_record(f'{service}.{proto}.local', query_type='TXT', expected=False, expect='key1=value1')
    dig_app.check_record(f'{service}.{proto}.local', query_type='TXT', expected=True, expect='key3=value3')
    dig_app.check_record(f'{service}.{proto}.local', query_type='TXT', expected=True, expect='key4=value4')


def test_update_txt(mdns_console, dig_app):
    check_txt_for_service('local', '_test', '_tcp', mdns_console=mdns_console, dig_app=dig_app)
    check_txt_for_service('local', '_test', '_tcp', mdns_console=mdns_console, dig_app=dig_app, with_inst=True)


def test_update_delegated_txt(mdns_console, dig_app):
    check_txt_for_service('extern', '_test2', '_tcp', mdns_console=mdns_console, dig_app=dig_app, host='delegated')
    check_txt_for_service('extern', '_test2', '_tcp', mdns_console=mdns_console, dig_app=dig_app, host='delegated', with_inst=True)


def test_service_port_set(mdns_console, dig_app):
    dig_app.check_record('local._test._tcp.local', query_type='SRV', expected=True, expect='80')
    mdns_console.send_input('mdns_service_port_set _test _tcp 81')
    dig_app.check_record('local._test._tcp.local', query_type='SRV', expected=True, expect='81')
    mdns_console.send_input('mdns_service_port_set _test2 _tcp -h delegated 82')
    dig_app.check_record('extern._test2._tcp.local', query_type='SRV', expected=True, expect='82')
    mdns_console.send_input('mdns_service_port_set _test2 _tcp -h delegated -i extern 83')
    dig_app.check_record('extern._test2._tcp.local', query_type='SRV', expected=True, expect='83')
    mdns_console.send_input('mdns_service_port_set _test2 _tcp -h delegated -i invalid_inst 84')
    mdns_console.get_output('ESP_ERR_NOT_FOUND')
    dig_app.check_record('extern._test2._tcp.local', query_type='SRV', expected=True, expect='83')


def test_service_subtype(mdns_console, dig_app):
    dig_app.check_record('local._test._tcp.local', query_type='SRV', expected=True)
    mdns_console.send_input('mdns_service_subtype _test _tcp _subtest -i local')
    dig_app.check_record('_subtest._sub._test._tcp.local', query_type='PTR', expected=True)
    mdns_console.send_input('mdns_service_subtype _test2 _tcp _subtest2 -i extern -h delegated')
    dig_app.check_record('_subtest2._sub._test2._tcp.local', query_type='PTR', expected=True)


def test_service_set_instance(mdns_console, dig_app):
    dig_app.check_record('local._test._tcp.local', query_type='SRV', expected=True)
    mdns_console.send_input('mdns_service_instance_set _test _tcp local2')
    dig_app.check_record('local2._test._tcp.local', query_type='SRV', expected=True)
    mdns_console.send_input('mdns_service_instance_set _test2 _tcp extern2 -h delegated')
    mdns_console.send_input('mdns_service_lookup _test2 _tcp -d')
    mdns_console.get_output('PTR : extern2')
    dig_app.check_record('extern2._test2._tcp.local', query_type='SRV', expected=True)
    mdns_console.send_input('mdns_service_instance_set _test2 _tcp extern3 -h delegated -i extern')
    mdns_console.get_output('ESP_ERR_NOT_FOUND')


def test_service_remove_all(mdns_console, dig_app):
    mdns_console.send_input('mdns_service_remove_all')
    mdns_console.send_input('mdns_service_lookup _test2 _tcp -d')
    mdns_console.get_output('No results found!')
    mdns_console.send_input('mdns_service_lookup _test _tcp')
    mdns_console.get_output('No results found!')
    dig_app.check_record('_test._tcp.local', query_type='PTR', expected=False)


if __name__ == '__main__':
    pytest.main(['-s', 'test_mdns.py'])

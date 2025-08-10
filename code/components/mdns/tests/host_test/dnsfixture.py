# SPDX-FileCopyrightText: 2024 Espressif Systems (Shanghai) CO LTD
# SPDX-License-Identifier: Unlicense OR CC0-1.0
import logging
import re
import socket
import sys

import dns.message
import dns.query
import dns.rdataclass
import dns.rdatatype
import dns.resolver

# Configure logging
logging.basicConfig(level=logging.INFO)
logger = logging.getLogger(__name__)


class DnsPythonWrapper:
    def __init__(self, server='224.0.0.251', port=5353, retries=3):
        self.server = server
        self.port = port
        self.retries = retries

    def send_and_receive_query(self, query, timeout=3):
        logger.info(f'Sending DNS query to {self.server}:{self.port}')
        try:
            # Create a UDP socket
            with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as sock:
                sock.settimeout(timeout)

                # Send the DNS query
                query_data = query.to_wire()
                sock.sendto(query_data, (self.server, self.port))

                # Receive the DNS response
                response_data, _ = sock.recvfrom(512)  # 512 bytes is the typical size for a DNS response

                # Parse the response
                response = dns.message.from_wire(response_data)

                return response

        except socket.timeout as e:
            logger.warning(f'DNS query timed out: {e}')
            return None
        except dns.exception.DNSException as e:
            logger.error(f'DNS query failed: {e}')
        return None

    def run_query(self, name, query_type='PTR', timeout=3):
        logger.info(f'Running DNS query for {name} with type {query_type}')
        query = dns.message.make_query(name, dns.rdatatype.from_text(query_type), dns.rdataclass.IN)

        # Print the DNS question section
        logger.info(f'DNS question section: {query.question}')

        # Send and receive the DNS query
        response = None
        for attempt in range(1, self.retries + 1):
            logger.info(f'Attempt {attempt}/{self.retries}')
            response = self.send_and_receive_query(query, timeout)
            if response:
                break

        if response:
            logger.info(f'DNS query response:\n{response}')
        else:
            logger.warning('No response received or response was invalid.')

        return response

    def parse_answer_section(self, response, query_type):
        answers = []
        if response:
            for answer in response.answer:
                if dns.rdatatype.to_text(answer.rdtype) == query_type:
                    for item in answer.items:
                        full_answer = (
                            f'{answer.name} {answer.ttl} '
                            f'{dns.rdataclass.to_text(answer.rdclass)} '
                            f'{dns.rdatatype.to_text(answer.rdtype)} '
                            f'{item.to_text()}'
                        )
                        answers.append(full_answer)
        return answers

    def check_record(self, name, query_type, expected=True, expect=None):
        output = self.run_query(name, query_type=query_type)
        answers = self.parse_answer_section(output, query_type)
        logger.info(f'answers: {answers}')
        if expect is None:
            expect = name
        if expected:
            assert any(expect in answer for answer in answers), f"Expected record '{expect}' not found in answer section"
        else:
            assert not any(expect in answer for answer in answers), f"Unexpected record '{expect}' found in answer section"


if __name__ == '__main__':
    if len(sys.argv) < 3:
        print('Usage: python dns_fixture.py <query_type> <name>')
        sys.exit(1)

    query_type = sys.argv[1]
    name = sys.argv[2]
    ip_only = len(sys.argv) > 3 and sys.argv[3] == '--ip_only'
    if ip_only:
        logger.setLevel(logging.WARNING)

    dns_wrapper = DnsPythonWrapper()
    if query_type == 'X' and '.' in name:
        # Sends an IPv4 reverse query
        reversed_ip = '.'.join(reversed(name.split('.')))
        name = f'{reversed_ip}.in-addr.arpa'
        query_type = 'PTR'
    response = dns_wrapper.run_query(name, query_type=query_type)
    answers = dns_wrapper.parse_answer_section(response, query_type)

    if answers:
        for answer in answers:
            logger.info(f'DNS query response: {answer}')
            if ip_only:
                ipv4_pattern = re.compile(r'\b(?:\d{1,3}\.){3}\d{1,3}\b')
                ipv4_addresses = ipv4_pattern.findall(answer)
                if ipv4_addresses:
                    print(f"{', '.join(ipv4_addresses)}")
    else:
        logger.info(f'No response for {name} with query type {query_type}')
        exit(9)     # Same as dig timeout

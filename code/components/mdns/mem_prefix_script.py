# SPDX-FileCopyrightText: 2025 Espressif Systems (Shanghai) CO LTD
# SPDX-License-Identifier: Apache-2.0
import re
import sys

# Configurable prefix for memory functions
MDNS_MEM_PREFIX = 'mdns_mem_'  # Change this to modify the prefix


def add_prefix_to_mem_funcs(content):
    # List of memory functions to prefix
    mem_funcs = [
        'malloc',
        'calloc',
        'free',
        'strdup',
        'strndup'
    ]

    # Create regex pattern matching the memory functions but not already prefixed ones
    pattern = fr'(?<!{MDNS_MEM_PREFIX})(?<![\w])(' + '|'.join(mem_funcs) + r')(?=\s*\()'

    # Replace all occurrences with configured prefix
    modified = re.sub(pattern, fr'{MDNS_MEM_PREFIX}\1', content)

    return modified


def process_file(filename):
    try:
        # Read the file
        with open(filename, 'r') as f:
            content = f.read()

        # Add prefixes
        modified = add_prefix_to_mem_funcs(content)

        # Write back to file
        with open(filename, 'w') as f:
            f.write(modified)

        print(f'Successfully processed {filename}')

    except Exception as e:
        print(f'Error processing {filename}: {str(e)}')
        sys.exit(1)


if __name__ == '__main__':
    if len(sys.argv) != 2:
        print('Usage: python script.py <filename>')
        sys.exit(1)

    process_file(sys.argv[1])

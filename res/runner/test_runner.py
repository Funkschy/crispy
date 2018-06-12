#!/usr/bin/python3

import sys
import subprocess
import os
from pathlib import Path


num_errors = 0


def execute(executable, test_dir, result_dir):
    global num_errors

    for file_name in os.listdir(test_dir):
        result_path = Path(result_dir + '/' + file_name.replace('test', 'expected'))
        if not result_path.is_file():
            print("Warning: {} has no result file".format(file_name))
            continue

        result_file = result_path.open()
        proc = subprocess.Popen([executable, test_dir + '/' + file_name], stdout=subprocess.PIPE)

        line = proc.stdout.readline().decode('utf-8').rstrip()
        while line != '':
            expected = result_file.readline().rstrip()

            if str(line) != str(expected):
                print('[{}]: Expected {}, but got {}'.format(file_name, expected, line))
                num_errors += 1

            line = proc.stdout.readline().decode('utf-8').rstrip()

        expected = result_file.readline().rstrip()
        if expected != '':
            print('[{}]: Error, did not print {}'.format(file_name, expected))

        result_file.close()


def main():
    help_text = '''
    Usage: test_runner.py [Path to executable] [Path to test directory] [Path to expected result directory]
    '''

    if len(sys.argv) != 4:
        print(help_text)
        return

    executable = str(Path(sys.argv[1]).absolute())
    test_dir = str(Path(sys.argv[2]).absolute())
    result_dir = str(Path(sys.argv[3]).absolute())

    execute(executable, test_dir, result_dir)

    if num_errors == 0:
        print('All tests were successful')
    else:
        print('There were errors')



if __name__ == '__main__':
    main()

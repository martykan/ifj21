#!/usr/bin/python3

import subprocess
import os
from enum import Enum

class TestRes:
    OK = 0
    DIFF_OUT = 1
    TIMEOUT = 2
    DIFF_EXIT_CODE = 3

    def __init__(self, type, msg='', ref_out='', out='', ref_ec=0, ec=0):
        self.type = type
        self.msg = msg
        self.ref_out = ref_out
        self.out = out
        self.ref_ec = ref_ec
        self.ec = ec

def get_path(dir: str, file_name: str):
    return os.path.join(dir, file_name)

def get_input_file(test_file: str, test_input_dir: str):
    base_file_name = test_file[:-3] # remove file extension
    in_file = [file for file in os.listdir(test_input_dir) if file == f"{base_file_name}.txt"]
    return get_path(test_input_dir, in_file[0]) if len(in_file) > 0 else ""

def run_test(test_file: str, test_input_dir: str) -> TestRes:

    input_data_cmd = get_input_file(test_file, test_input_dir)
    input_data_cmd = f"<{input_data_cmd}" if len(input_data_cmd) > 0 else ""

    cmd = f'tl run {get_path(test_input_dir, test_file)}{input_data_cmd}'

    try:
        p = subprocess.Popen(cmd, stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE, shell=True)
        out, err = p.communicate(timeout=3)

        # TODO(filip): call our program

        # TODO(filip): compare outputs
        # TODO(filip): compare exit codes
        # print(out.decode(), err.decode(), end="") 
    except subprocess.TimeoutExpired as e:
        return TestRes(TestRes.TIMEOUT, msg=str(e))

    else:
        return TestRes(TestRes.OK)

def print_test_suite_header(dir: str):
    print(f"* Suite {dir}:")

def print_test_suite_footer(num_run: int, num_failed: int):
    print(f"\n{num_run} tests - {num_run - num_failed} passed, {num_failed} failed\n")

def print_test_fail(test_name: str, res: TestRes):
    test_res_str = ["OK", "OUT_DIFF", "TIMEOUT"]
    print(f"Fail - {test_name}: {test_res_str[res.type]}")
    if len(res.msg) != 0:
        print(res.msg)
    if res.type == TestRes.DIFF_OUT:
        print("Expected output:")
        print(res.ref_out)
        print("----------------------\n\n", end="")
        print("Got output:")
        print(res.out)


def print_test_success():
    print(".", end="")

def print_final_footer(total_tests: int, total_failed: int):
    print(f"Total: {total_tests} tests - {total_tests - total_failed} passed, {total_failed} failed")

def all_tests():
    """ Run all tests from example-code folder.
    This script has to be executed in a parent folder of example-code."""
    cwd = os.getcwd()
    test_input_dir = get_path(cwd, "example-code")
    dirs = [dir for dir in os.listdir(test_input_dir) if os.path.isdir(os.path.join(test_input_dir, dir))]

    total_tests = 0
    total_failed = 0
    for dir in dirs:
        num_tests_in_suite = 0
        num_tests_in_suite_fail = 0

        tl_files = [file for file in os.listdir(get_path(test_input_dir, dir)) if file[-3:] == ".tl"]
        print_test_suite_header(dir)

        for test_file in tl_files:
            res = run_test(test_file, get_path(test_input_dir, dir))
            num_tests_in_suite += 1
            if res.type != TestRes.OK:
                num_tests_in_suite_fail += 1
                print_test_fail(test_file, res)
            else:
                print_test_success()

        print_test_suite_footer(num_tests_in_suite, num_tests_in_suite_fail)
        total_tests += num_tests_in_suite
        total_failed += num_tests_in_suite_fail

    print_final_footer(total_tests, total_failed)

    if total_failed != 0:
        exit(1)


if __name__ == "__main__":
    all_tests()


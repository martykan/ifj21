#!/usr/bin/python3

import subprocess
import os


class TestRes:
    OK = 0
    DIFF_OUT = 1
    TIMEOUT = 2
    DIFF_EXIT_CODE = 3
    SKIPPED = 4
    INVALID_INPUT = 5
    INTERNAL = 6
    RUNTIME_ERROR = 7

    def __init__(self, type, msg="", ref_out="", out="", ref_ec=0, ec=0):
        self.type = type
        self.msg = msg
        self.ref_out = ref_out
        self.out = out
        self.ref_ec = ref_ec
        self.ec = ec


def get_path(dir: str, file_name: str):
    return os.path.join(dir, file_name)


def get_input_file(test_file: str, test_input_dir: str):
    base_file_name = test_file[:-3]  # remove file extension
    in_file = [
        file for file in os.listdir(test_input_dir) if file == f"{base_file_name}.txt"
    ]
    return get_path(test_input_dir, in_file[0]) if len(in_file) > 0 else ""


def run_test(test_file: str, test_input_dir: str) -> TestRes:
    if os.path.getsize(get_path(test_input_dir, test_file)) <= 0:
        return TestRes(TestRes.SKIPPED)

    input_data_cmd = get_input_file(test_file, test_input_dir)
    input_data_cmd = f"<{input_data_cmd}" if len(input_data_cmd) > 0 else ""

    try:
        expected_rc = int(test_file[:test_file.index("_")])
    except ValueError as e:
        return TestRes(TestRes.INTERNAL, msg=f"Invalid test file name: {test_file}");

    ref_cmd = f"tl run {get_path(test_input_dir, test_file)}{input_data_cmd}"
    comp_cmd = f"../../ifj21 < {get_path(test_input_dir, test_file)}"

    try:
        ref = subprocess.Popen(
            ref_cmd,
            stdin=subprocess.PIPE,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            shell=True,
        )
        ref_out, ref_err = ref.communicate(timeout=3)
        ref_rc = ref.returncode

        comp = subprocess.Popen(
            comp_cmd,
            stdin=subprocess.PIPE,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            shell=True,
        )

        comp_out, comp_err = comp.communicate(timeout=3)
        comp_rc = comp.returncode

        comp_out_file = f"out_{test_file}"
        f = open(comp_out_file, "w")
        f.write(comp_out.decode())
        f.close()

        if expected_rc != comp_rc:
            return TestRes(TestRes.DIFF_EXIT_CODE, msg=f"Expected exit code doesn't match actual:\nexpected: {expected_rc}, got: {comp_rc}\n{comp_err.decode()}")

        # interpret generated code
        if expected_rc == 0:
            inter_cmd = f"./ic21int {comp_out_file}{input_data_cmd}"
            inter = subprocess.Popen(
                inter_cmd,
                stdin=subprocess.PIPE,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                shell=True,
            )
            inter_out, inter_err = inter.communicate(timeout=3)
            if inter.returncode != 0:
                return TestRes(TestRes.RUNTIME_ERROR, msg=f"Interpreter reported a runtime error: {inter.returncode}\n{inter_err.decode()}")

            if inter_out.decode() != ref_out.decode():
                return TestRes(TestRes.DIFF_OUT, msg=f"Outputs don't match:\nExpected:\n{ref_out.decode()}\n\nGot:\n{inter_out.decode()}")

    except subprocess.TimeoutExpired as e:
        return TestRes(TestRes.TIMEOUT, msg=str(e))

    else:
        return TestRes(TestRes.OK)


def print_test_suite_header(dir: str):
    print(f"* Suite {dir}:")


def print_test_suite_footer(num_passed: int, num_failed: int, num_skipped: int):
    print(
        f"\n{num_passed + num_failed} tests run - {num_passed} passed, {num_failed} failed, {num_skipped} skipped\n"
    )


def print_test_fail(test_name: str, res: TestRes):

    test_res_str = ["OK", "DIFF_OUT", "TIMEOUT", "DIFF_EXIT_CODE", "SKIPPED", "INVALID_INPUT", "INTERNAL", "RUNTIME_ERROR"]
    print(f"\nFail - {test_name}: {test_res_str[res.type]}")
    if len(res.msg) != 0:
        print(res.msg)
    # if res.type == TestRes.DIFF_OUT:
    #     print("Expected output:")
    #     print(res.ref_out)
    #     print("----------------------\n\n", end="")
    #     print("Got output:")
    #     print(res.out)

    print("\n", end="")


def print_test_success():
    print(".", end="")


def print_final_footer(total_passed: int, total_failed: int, total_skipped: int):
    print(
        f"Total: {total_passed + total_failed} tests run - {total_passed} passed, {total_failed} failed, {total_skipped} skipped"
    )


def all_tests():
    """Run all tests from example-code folder.
    This script has to be executed in a parent folder of example-code."""
    cwd = os.getcwd()
    test_input_dir = get_path(cwd, "example-code")
    dirs = [
        dir
        for dir in os.listdir(test_input_dir)
        if os.path.isdir(os.path.join(test_input_dir, dir))
    ]

    total_tests = 0
    total_failed = 0
    total_skipped = 0
    for dir in dirs:
        num_tests_in_suite = 0
        num_tests_in_suite_fail = 0
        num_tests_in_suite_skipped = 0

        tl_files = [
            file
            for file in os.listdir(get_path(test_input_dir, dir))
            if file[-3:] == ".tl"
        ]
        print_test_suite_header(dir)

        for test_file in tl_files:
            curr_test_dir = get_path(test_input_dir, dir)
            res = run_test(test_file, curr_test_dir)
            if res.type == TestRes.SKIPPED:
                num_tests_in_suite_skipped += 1
            elif res.type != TestRes.OK:
                num_tests_in_suite_fail += 1
                print_test_fail(test_file, res)
            else:
                num_tests_in_suite += 1
                if (os.path.isfile(get_path(cwd, f"out_{test_file}"))):
                    os.remove(get_path(cwd, f"out_{test_file}"))
                print_test_success()

        print_test_suite_footer(
            num_tests_in_suite, num_tests_in_suite_fail, num_tests_in_suite_skipped
        )
        total_tests += num_tests_in_suite
        total_failed += num_tests_in_suite_fail
        total_skipped += num_tests_in_suite_skipped

    print_final_footer(total_tests, total_failed, total_skipped)

    if total_failed != 0:
        exit(1)


if __name__ == "__main__":
    all_tests()

#!/usr/bin/env python3

from pathlib import Path
import os
import subprocess
import re

from enum import Enum
import sys
from typing import Optional

TESTEXE = "build/Debug/address/src/cpplox"


EXPECTED_OUTPUT_PATTERN = re.compile(r"// expect: ?(.*)")
EXPECTED_ERROR_PATTERN = re.compile(r"// (Error.*)")
ERROR_LINE_PATTERN = re.compile(r"// \[((java|c) )?line (\d+)\] (Error.*)")
EXPECTED_RUNTIME_ERROR_PATTERN = re.compile(r"// expect runtime error: (.+)")
SYNTAX_ERROR_PATTERN = re.compile(r"\[.*line (\d+)\] (Error.+)")
STACKTRACE_PATTERN = re.compile(r"\[line (\d+)\]")
NONTEST_PATTERN = re.compile(r"// nontest")


class TERMCOLORS:
    GREEN = "\033[92m"
    RED = "\033[91m"
    RESET = "\033[0m"


class ResultType(Enum):
    SUCCESS = 0
    FAIL = 1
    SKIPPED = 2


class Testcase:
    class ExpectedOutput:
        line: int
        output: str

        def __init__(self, l: int, o: str) -> None:
            self.line = l
            self.output = o

    path: Path
    expected_output: list[ExpectedOutput]
    expected_errors: list[str]
    expected_runtime_error: str
    expected_runtime_error_line: int
    expected_exit_code: int
    failures: list[str]

    def __init__(self, p: Path):
        self.path = p
        self.expected_output = []
        self.expected_errors = []
        self.expected_runtime_error = ""
        self.expected_runtime_error_line = 0
        self.expected_exit_code = 0
        self.failures = []

    def fail(self, msg: str, lines: list[str] = []):
        self.failures.append(msg)

        for l in lines:
            self.failures.append(l)


class TestResult:
    result: ResultType
    msg: str
    errors: list[str]

    def __init__(self, res: ResultType, msg: str, errors: list[str]):
        self.msg = msg
        self.result = res
        self.errors = errors


class TestRunner:
    def __init__(self) -> None:
        self.passed = 0
        self.failed = 0
        self.skipped = 0
        self.testcases: list[Testcase] = []

    def find_testcases(self):
        for root, _, files in os.walk("tests"):
            for f in files:
                if "benchmark" in root:
                    self.skipped += 1
                elif Path(f).suffix == ".lox":
                    tc = parse(Path(root, f))

                    if tc == None:
                        raise Exception(
                            f"Testcase {Path(root, f)} could not be parsed!"
                        )
                    else:
                        self.testcases.append(tc)

    def run_tests(self):
        for tc in self.testcases:

            sys.stdout.write("\033[K")  # Clear to the end of line
            print(f"{tc.path}", end="\r")
            failures = run(tc)

            if len(failures) == 0:
                self.passed += 1
            else:
                self.failed += 1

                print("")
                print(f"{TERMCOLORS.RED}FAIL{TERMCOLORS.RESET} {tc.path}")
                for l in failures:
                    print("     " + l)

        print("")

    def print_summary(self):

        print("total cases:", len(self.testcases) + self.skipped)
        print(f"{TERMCOLORS.GREEN}passed: {self.passed}{TERMCOLORS.RESET}")
        if self.failed == 0:
            print(f"{TERMCOLORS.GREEN}failed: {self.failed}{TERMCOLORS.RESET}")
        else:
            print(f"{TERMCOLORS.RED}failed: {self.failed}{TERMCOLORS.RESET}")
        print("skipped:", self.skipped)


def parse(p: Path) -> Optional[Testcase]:
    t = Testcase(p)

    with open(p) as f:
        lines = f.readlines()

        linenum = 1
        while linenum <= len(lines):
            line = lines[linenum - 1]

            # Not a test file at all, so ignore it.

            match = NONTEST_PATTERN.search(line)
            if match != None:
                return None

            match = EXPECTED_OUTPUT_PATTERN.search(line)
            if match != None:
                t.expected_output.append(Testcase.ExpectedOutput(linenum, match[1]))
                linenum += 1
                continue

            match = EXPECTED_ERROR_PATTERN.search(line)
            if match != None:
                t.expected_errors.append(f"[{linenum}] {match[1]}")
                t.expected_exit_code = 65
                linenum += 1
                continue

            match = ERROR_LINE_PATTERN.search(line)
            if match != None:
                t.expected_errors.append(f"[{match[3]}] {match[4]}")
                t.expected_exit_code = 65
                linenum += 1
                continue

            match = EXPECTED_RUNTIME_ERROR_PATTERN.search(line)
            if match != None:
                t.expected_runtime_error_line = linenum
                t.expected_runtime_error = match[1]
                t.expected_exit_code = 70

            linenum += 1

    if len(t.expected_errors) > 0 and t.expected_runtime_error != "":
        raise Exception(str(p) + ": Cannot expect both compile and runtime errors.")

    return t


def run(tc: Testcase) -> list[str]:
    res = subprocess.run(
        [TESTEXE, tc.path], capture_output=True, text=True, encoding="utf8"
    )

    output = str(res.stdout).splitlines()
    errors = str(res.stderr).splitlines()

    if tc.expected_runtime_error != "":
        validate_runtime_error(tc, errors)
    else:
        validate_compile_errors(tc, errors)

    validate_exit_code(tc, res.returncode, errors)
    validate_output(tc, output)

    return tc.failures


def validate_runtime_error(tc: Testcase, errors: list[str]):
    if len(errors) < 2:
        tc.fail(f"Expected runtime error {tc.expected_runtime_error} and got none.")
        return

    if errors[0] != tc.expected_runtime_error:
        tc.fail(
            f"Expected runtime error {tc.expected_runtime_error} and got: ",
            errors,
        )

    # Make sure the stack trace has the right line.
    stacklines = errors[1:]
    match = None
    for l in stacklines:
        match = STACKTRACE_PATTERN.search(l)

        if match != None:
            break

    if match == None:
        tc.fail("Expected stack trace and got:", stacklines)
    else:
        stackline = int(match[1])
        if stackline != tc.expected_runtime_error_line:
            tc.fail(
                f"Expected runtime error on line {tc.expected_runtime_error_line}, but was on line {stackline}"
            )


def validate_compile_errors(tc: Testcase, errors: list[str]):
    # Validate that every compile error was expected.
    founderrors = list[str]()
    unexpectedErrorCount = 0

    for line in errors:
        match = SYNTAX_ERROR_PATTERN.search(line)
        if match != None:
            err = f"[{match[1]}] {match[2]}"
            if err in tc.expected_errors:
                founderrors.append(err)
            else:
                if unexpectedErrorCount < 10:
                    tc.fail("Unexpected error:", [line])
                unexpectedErrorCount += 1
        elif line != "":
            if unexpectedErrorCount < 10:
                tc.fail("Unexpected error:", [line])
            unexpectedErrorCount += 1

    if unexpectedErrorCount > 10:
        tc.fail(f"(truncated {unexpectedErrorCount-10} more...)")

    # Validate that every expected error occurred.
    for error in set(tc.expected_errors).difference(founderrors):
        tc.fail(f"missing expected error: {error}")


def validate_exit_code(tc: Testcase, exit_code: int, errors: list[str]):
    if exit_code == tc.expected_exit_code:
        return

    if len(errors) > 10:
        errors = errors[0:10]
        errors.append("(truncated...)")

    tc.fail(
        f"Expected return code {tc.expected_exit_code} and got {exit_code}. stderr:",
        errors,
    )


def validate_output(tc: Testcase, output: list[str]):
    # Remove the trailing last empty line.

    if len(output) > 0 and output[-1] == "":
        output.pop()

    index = 0

    while index < len(output):
        line = output[index]

        if index >= len(tc.expected_output):
            tc.fail(f"Got output {line} when none was expected.")
            index += 1
            continue

        expected = tc.expected_output[index]
        if expected.output != line:
            tc.fail(
                f"Expected output '{expected.output}' on line {expected.line} and got '{line}'."
            )
        index += 1

    while index < len(tc.expected_output):
        expected = tc.expected_output[index]
        tc.fail(f"Missing expected output '{expected.output}' on line {expected.line}.")
        index += 1


def main():

    runner = TestRunner()

    runner.find_testcases()

    runner.run_tests()

    runner.print_summary()


if __name__ == "__main__":
    main()

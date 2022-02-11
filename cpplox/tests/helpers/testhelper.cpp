#include <algorithm>
#include <iostream>
#include <iterator>
#include <optional>
#include <regex>
#include <sstream>
#include <string>
#include <vector>

#include "testhelper.h"

#include "fmt/core.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "lox/vm.h"

using namespace std;

namespace
{

template<typename T>
std::vector<T> pop_front(std::vector<T> v)
{
  if (v.size() <= 1) {
    return std::vector<T> {};
  }

  std::vector<T> res;

  std::copy(v.cbegin() + 1, v.cend(), back_inserter(res));

  return res;
}

struct RuntimeError
{
  std::string what;
};

struct CompileError
{
  std::string line;
  std::string what;
};

struct Result
{
  InterpretResult result;
  std::string stdoutOutput;
  std::string stderrOutput;
};

Result run_impl(const char* source)
{
  std::stringstream stderrstream, stdoutstream;

  auto oldstdout = std::cout.rdbuf();
  auto oldstderr = std::cerr.rdbuf();

  std::cout.rdbuf(stdoutstream.rdbuf());
  std::cerr.rdbuf(stderrstream.rdbuf());

  VM vm;

  auto res = vm.interpret(source);

  std::cout.rdbuf(oldstdout);
  std::cerr.rdbuf(oldstderr);

  return Result {res, stdoutstream.str(), stderrstream.str()};
}

std::vector<std::string> splitLines(std::string output)
{
  std::vector<std::string> res;
  std::stringstream stream(output);

  std::string to;
  while (std::getline(stream, to, '\n')) {
    res.push_back(to);
  }

  return res;
}

std::vector<std::string> stripDebugOutput(std::vector<std::string> in)
{
  std::vector<std::string> res;

  for (auto str : in) {
    if (!(str.rfind("DBG:", 0) == 0)) {
      res.push_back(str);
    }
  }

  return res;
}

std::optional<RuntimeError> getExpectedRuntimeError(std::string source)
{
  std::optional<RuntimeError> res;

  // expect runtime error: Undefined variable 'unknown'.

  const auto lines = splitLines(source);
  for (size_t i = 0; i < lines.size(); i++) {
    const auto line = lines[i];

    std::regex rgx {"// expect runtime error: (.*)"};
    std::smatch match;

    if (std::regex_search(line.cbegin(), line.cend(), match, rgx)) {
      const auto what = match[1];

      return RuntimeError {what};
    }
  }

  return std::nullopt;
}

std::vector<std::string> getExpectedOutput(std::string source)
{
  std::vector<std::string> res;

  auto lines = splitLines(source);

  for (auto line : lines) {
    std::regex rgx {"// expect: (.*)$"};
    std::smatch match;

    if (std::regex_search(line.cbegin(), line.cend(), match, rgx)) {
      res.push_back(match[1]);
    }
  }

  return res;
}

std::vector<CompileError> getExpectedCompileErrors(std::string source)
{
  std::vector<CompileError> res;
  auto lines = splitLines(source);

  for (size_t i = 0; i < lines.size(); i++) {
    const auto line = lines[i];

    std::regex rgx {"// ?(.*)? (Error.*)"};  // pattern for any compile error
    std::smatch match;

    if (std::regex_search(line.cbegin(), line.cend(), match, rgx)) {
      const auto lineNr = match[1];
      const auto what = match[2];

      res.push_back(CompileError {lineNr, what});
    }
  }

  return res;
}

void checkStackTrace(std::vector<std::string> stderrOutput)
{
  ASSERT_GE(stderrOutput.size(), 2);  // stacktrace needs to be at least 2 lines

  bool first = true;

  for (auto line : stderrOutput) {
    if (first) {
      first = false;
      continue;
    }

    std::regex rgx {".*"};
    if (!regex_match(line, rgx)) {
      ADD_FAILURE() << "Line " << line << " does not fit stacktrace pattern.";
    }
  }
}

void checkRuntimeError(std::optional<RuntimeError> expected,
                       std::vector<std::string> stderrOutput)
{
  if (expected.has_value()) {
    ASSERT_GE(stderrOutput.size(), 2)
        << "Expected runtime error, but stderr ouput is too small.";
  } else {
    ASSERT_EQ(stderrOutput.size(), 0)
        << "Did not expect runtime error, but stderr ouput is not 0!"
        << fmt::format("{}", fmt::join(stderrOutput, "\n"));
  }

  for (size_t i = 0; i < stderrOutput.size(); i++) {
    const auto line = stderrOutput[i];
    std::regex rgx {"line (.*)] in (.*)"};
    std::smatch match;
    if (std::regex_search(line.cbegin(), line.cend(), match, rgx)) {
      const auto in = match[2];

      if (i == 0) {
        FAIL() << "Encountered runtime error, but there is no error message.";
      }

      const auto prevLine = stderrOutput[i - 1];

      if (!expected.has_value()) {
        ADD_FAILURE() << "Received an unexpected runtime error in function "
                      << in << " with error message " << prevLine;
        continue;
      }

      EXPECT_EQ(expected->what, prevLine)
          << "Expected error message " << expected->what << " but got"
          << prevLine;

      checkStackTrace(stderrOutput);
      return;
    }
  }
}

void checkExpectedOutput(std::vector<std::string> expected,
                         std::vector<std::string> actual)
{
  if (expected.size() == actual.size()) {
    for (size_t i = 0; i < expected.size(); i++) {
      ASSERT_EQ(expected.at(i), actual.at(i))
          << "Expected output " << expected.at(i) << " and got "
          << actual.at(i);
    }
  } else if (expected.size() < actual.size()) {
    for (size_t i = expected.size(); i < actual.size(); i++) {
      ADD_FAILURE() << "Received unexpected output " << actual[i];
    }
  } else {
    ASSERT_LT(actual.size(), expected.size()) << "Wait, what????";

    for (size_t i = actual.size(); i < expected.size(); i++) {
      ADD_FAILURE() << "Never received expected output " << expected[i];
    }
  }
}

void checkCompileError(std::vector<CompileError> expected,
                       std::vector<std::string> stderrOutput)
{
  if (expected.size() > 0) {
    ASSERT_GT(stderrOutput.size(), 0)
        << "Expected compiler error, but there is no stderr ouput.";
  }

  for (size_t i = 0; i < stderrOutput.size(); i++) {
    const auto line = stderrOutput[i];
    std::regex rgx {"(.*)? ?(Error.*)"};
    std::smatch match;
    if (std::regex_search(line.cbegin(), line.cend(), match, rgx)) {
      const auto lineNr = match[1];
      const auto what = match[2];

      if (expected.empty()) {
        ADD_FAILURE() << "Received unexpected compile error! Line: " << lineNr
                      << "; What: " << what;
        continue;
      }

      const auto exp = expected.front();
      expected = pop_front(expected);

      EXPECT_EQ(exp.what, what)
          << "Expected error message " << exp.what << " but got" << what;
    }
  }

  if (expected.size() != 0) {
    std::cout << "Never received the following errors:" << endl;

    for (auto err : expected) {
      cout << "Line: " << err.line << "; What: " << err.what << endl;
    }

    ADD_FAILURE() << "Not all expected errors received!";
  }
}
}  // namespace

void End2EndTest::run(const char* source)
{
  using ::testing::HasSubstr;

  const auto expectedOutput = getExpectedOutput(source);
  const auto expectedCompileErrors = getExpectedCompileErrors(source);
  const auto expectedRuntimeError = getExpectedRuntimeError(source);

  if (expectedRuntimeError.has_value() && !expectedCompileErrors.empty()) {
    FAIL() << "Cannot have both expected runtime error and expected compile "
              "error!";
    return;
  }

  const auto res = run_impl(source);

  const auto so = stripDebugOutput(splitLines(res.stdoutOutput));
  const auto se = stripDebugOutput(splitLines(res.stderrOutput));

  std::cout << "============= STDOUT ===============\n";
  for (auto line : so) {
    std::cout << line << endl;
  }

  std::cout << "============= STDERR ===============\n";
  for (auto line : se) {
    std::cout << line << endl;
  }
  std::cout << "====================================\n";

  if (expectedCompileErrors.empty() && !expectedRuntimeError.has_value()) {
    ASSERT_EQ(se.size(), 0) << "No runtime or compile errors expected, but "
                               "stderr output is not empty!";
  }

  const auto actual = res.result;
  if (!expectedCompileErrors.empty()) {
    const auto expected = InterpretResult::COMPILE_ERROR;
    EXPECT_EQ(actual, expected);
    checkCompileError(expectedCompileErrors, se);
  } else if (expectedRuntimeError.has_value()) {
    const auto expected = InterpretResult::RUNTIME_ERROR;
    EXPECT_EQ(actual, expected);
    checkRuntimeError(expectedRuntimeError, se);
  } else {
    const auto expected = InterpretResult::OK;
    EXPECT_EQ(actual, expected);
  }

  checkExpectedOutput(expectedOutput, so);
}

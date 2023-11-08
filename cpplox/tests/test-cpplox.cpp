#include <algorithm>
#include <deque>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <memory>
#include <optional>
#include <regex>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

#include <fmt/format.h>
#include <gtest/gtest.h>

#include "vm.h"

using namespace std;

namespace
{
const auto expectedOutputPattern = ("// expect: ?(.*)");
const auto expectedErrorPattern = ("// (Error.*)");
const auto errorLinePattern = ("// \\[((java|c) )?line (\\d+)\\] (Error.*)");
const auto expectedRuntimeErrorPattern = ("// expect runtime error: (.+)");
const auto syntaxErrorPattern = ("\\[line (\\d+)\\] (Error.+)");
const auto stackTracePattern = ("\\[line (\\d+)\\]");
const auto nonTestPattern = ("// nontest");
}  // namespace

struct RawTestOutput
{
  InterpretResult result;
  deque<string> stdoutOutput;
  deque<string> stderrOutput;
};

struct ExpectedTestResult
{
  string file_name;

  // pair of line number and output
  deque<pair<size_t, string>> expected_output;

  // compiler errors
  deque<string> expected_errors;

  optional<string> expected_runtime_error;

  size_t expected_runtime_error_line;
};

deque<string> split_newlines(string output)
{
  deque<std::string> res;
  stringstream stream(output);

  string to;
  while (getline(stream, to, '\n')) {
    res.push_back(to);
  }

  return res;
}

RawTestOutput run_impl(string source)
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

  return RawTestOutput {res,
                        split_newlines(stdoutstream.str()),
                        split_newlines(stderrstream.str())};
}

optional<string> parse_expected_output(string line)
{
  regex rgx {expectedOutputPattern};
  smatch match;

  if (regex_search(line.cbegin(), line.cend(), match, rgx)) {
    return match[1];
  }

  return nullopt;
}

optional<string> parse_runtime_error(string line)
{
  std::regex rgx {expectedRuntimeErrorPattern};
  std::smatch match;

  if (std::regex_search(line.cbegin(), line.cend(), match, rgx)) {
    return {match[1]};
  }

  return std::nullopt;
}

optional<string> parse_error_pattern(string line)
{
  std::regex rgx {expectedErrorPattern};
  std::smatch match;

  if (std::regex_search(line.cbegin(), line.cend(), match, rgx)) {
    return std::string {match[1]};
  }

  return nullopt;
}

optional<string> parse_error_line_pattern(string line)
{
  std::regex rgx {errorLinePattern};
  std::smatch match;

  if (std::regex_search(line.cbegin(), line.cend(), match, rgx)) {
    return fmt::format("[line {}] {}", string {match[3]}, string {match[4]});
  }

  return nullopt;
}

ExpectedTestResult parse_expected_output_from_lox_source(string lox_source)
{
  ExpectedTestResult result;

  auto lines = split_newlines(lox_source);

  for (size_t linenr = 1; linenr <= lines.size(); linenr++) {
    const auto line = lines.at(linenr - 1);

    // check line for output
    if (auto output = parse_expected_output(line)) {
      result.expected_output.push_back(pair {linenr, *output});
      continue;
    }

    // check line for errors
    if (auto compile_error = parse_error_pattern(line)) {
      result.expected_errors.push_back(
          fmt::format("[line {}] {}", linenr, *compile_error));

      continue;
    }

    if (auto match = parse_error_line_pattern(line)) {
      result.expected_errors.push_back(*match);
      continue;
    }

    if (auto runtime_error = parse_runtime_error(line)) {
      EXPECT_FALSE(result.expected_runtime_error.has_value());
      result.expected_runtime_error = runtime_error;
      result.expected_runtime_error_line = linenr;
    }
  }

  return result;
}

// optional<string> parse_actual_output(string line)
// {
//   if (line.empty()) {
//     return nullopt;
//   }

//   return line;
// }

// optional<string> parse_actual_runtime_error(string prev_line, string line)
// {
//   std::regex rgx {"line (.*)] in (.*)"};
//   std::smatch match;
//   if (std::regex_search(line.cbegin(), line.cend(), match, rgx)) {
//     const string linenr = match[1];
//     const string in = match[2];

//     return {prev_line};
//   }

//   return nullopt;
// }

// optional<string> parse_actual_compile_error(string line)
// {
//   std::regex rgx {"(.*)? ?(Error.*)"};
//   std::smatch match;
//   if (std::regex_search(line.cbegin(), line.cend(), match, rgx)) {
//     const string lineNr = match[1];
//     const string what = match[2];

//     return {what};
//   }

//   return nullopt;
// }

/*
final _expectedOutputPattern = RegExp(r"// expect: ?(.*)");
final _expectedErrorPattern = RegExp(r"// (Error.*)");
final _errorLinePattern = RegExp(r"// \[((java|c) )?line (\d+)\] (Error.*)");
final _expectedRuntimeErrorPattern = RegExp(r"// expect runtime error: (.+)");
final _syntaxErrorPattern = RegExp(r"\[.*line (\d+)\] (Error.+)");
final _stackTracePattern = RegExp(r"\[line (\d+)\]");
final _nonTestPattern = RegExp(r"// nontest");
*/

void validate_runtime_error(RawTestOutput output, ExpectedTestResult expected)
{
  auto errorlines = output.stderrOutput;

  ASSERT_TRUE(expected.expected_runtime_error.has_value());

  auto expected_runtime_error = *expected.expected_runtime_error;

  ASSERT_GE(errorlines.size(), 2) << fmt::format(
      "Expected runtime error {} and got none.", expected_runtime_error);

  ASSERT_EQ(expected_runtime_error, errorlines[0])
      << fmt::format("Expected runtime error '{}' and got '{}'.",
                     expected_runtime_error,
                     errorlines[0]);

  // TODO: parse stacktrace and verify
  // * correct stack trace pattern
  // * line numbers of stacktrace match
}

bool contains(const deque<string>& haystack, const string& needle)
{
  auto it = find(cbegin(haystack), cend(haystack), needle);

  if (it == cend(haystack)) {
    return false;
  }

  return true;
}

deque<string> difference(deque<string> lhs, deque<string> rhs)
{
  deque<string> res;
  copy_if(cbegin(lhs),
          cend(lhs),
          back_inserter(res),
          [&res, &rhs](const string& elem) {
            if (contains(res, elem)) {
              return false;
            }

            if (contains(rhs, elem)) {
              return false;
            }
            return true;
          });

  copy_if(cbegin(rhs),
          cend(rhs),
          back_inserter(res),
          [&res, &lhs](const string& elem) {
            if (contains(res, elem)) {
              return false;
            }

            if (contains(lhs, elem)) {
              return false;
            }
            return true;
          });

  return res;
}

void validate_compile_error(RawTestOutput output, ExpectedTestResult expected)
{
  // Validate that every compile error was expected.
  deque<string> found_errors;
  int unexpected_count = 0;

  for (string line : output.stderrOutput) {
    std::regex rgx {syntaxErrorPattern};
    std::smatch match;

    if (std::regex_search(line, match, rgx)) {
      const string lineNr = match[1];
      const string what = match[2];

      auto error = fmt::format("[line {}] {}", lineNr, what);

      if (contains(expected.expected_errors, error)) {
        found_errors.push_back(error);
      } else {
        if (unexpected_count < 10) {
          ADD_FAILURE() << fmt::format("Unexpected error: {}", line);
        }

        unexpected_count++;
      }

    } else if (line != "") {
      if (unexpected_count < 10) {
        ADD_FAILURE() << fmt::format("Unexpected output on stderr: {}", line);
      }

      unexpected_count++;
    }
  }

  if (unexpected_count > 10) {
    ADD_FAILURE() << fmt::format("truncated {} more...", unexpected_count - 10);
  }

  // Validate that every expected error occurred.
  auto diff = difference(expected.expected_errors, found_errors);
  for (auto error : diff) {
    ADD_FAILURE() << fmt::format("Missing expected error: {}", error);
  }
}

void validate_output(RawTestOutput output, ExpectedTestResult expected)
{
  auto output_lines = output.stdoutOutput;

  if (!output_lines.empty() && output_lines.back() == "") {
    // remove trailing empty line
    output_lines.pop_back();
  }

  auto index = 0u;

  for (; index < output_lines.size(); index++) {
    string line = output_lines[index];

    ASSERT_LT(index, expected.expected_output.size());

    auto exp = expected.expected_output[index];

    EXPECT_EQ(exp.second, line);
  }

  while (index < expected.expected_output.size()) {
    auto exp = expected.expected_output[index];

    ADD_FAILURE() << fmt::format(
        "Missing expected output '{}' on line {}", exp.second, exp.first);
    index++;
  }
}

void check_results(ExpectedTestResult expected, RawTestOutput output)
{
  // runtime and compile time error may not happen at the same time
  ASSERT_FALSE(expected.expected_runtime_error.has_value()
               && !expected.expected_errors.empty());

  if (expected.expected_runtime_error.has_value()) {
    validate_runtime_error(output, expected);
  } else {
    validate_compile_error(output, expected);
  }

  validate_output(output, expected);
}

void run(string source)
{
  const auto expected_output = parse_expected_output_from_lox_source(source);

  const auto res = run_impl(source);

  check_results(expected_output, res);
}

class LoxTextFixture : public testing::Test
{
public:
  // All of these optional, just like in regular macro usage.
  static void SetUpTestSuite() {}
  static void TearDownTestSuite() {}
  void SetUp() override {}
  void TearDown() override {}
};

class LoxTest : public LoxTextFixture
{
public:
  explicit LoxTest(string filepath)
      : m_file_path(filepath)
  {
  }

  inline std::optional<string> read_file(string path)
  {
    constexpr auto read_size = 4096u;
    auto stream = ifstream(path.data());
    stream.exceptions(ios_base::badbit);

    if (!stream) {
      return nullopt;
    }

    auto out = std::string();
    auto buf = std::string(read_size, '\0');
    while (stream.read(&buf[0], read_size)) {
      out.append(buf, 0, stream.gcount());
    }
    out.append(buf, 0, stream.gcount());
    return out;
  }

  inline

      void
      TestBody() override
  {
    VM vm;

    auto source = read_file(m_file_path);

    ASSERT_TRUE(source.has_value());

    run(*source);
  }

private:
  string m_file_path;
};

struct TestData
{
  string test_suite;
  string test_name;
};

void register_tests(const vector<string>& paths)
{
  for (auto path : paths) {
    testing::RegisterTest(
        "MyFixture",
        ("Test" + (path)).c_str(),
        nullptr,
        nullptr,
        (path).c_str(),
        __LINE__,
        // Important to use the fixture type as the return type here.
        [=]() -> LoxTextFixture* { return new LoxTest(path); });
  }
}

vector<string> get_all_tests()
{
  vector<string> ret;

  for (auto entry : filesystem::recursive_directory_iterator("test")) {
    auto str = entry.path().string();

    if (str.find("benchmark") != string::npos) {
      continue;
    }

    if (entry.is_regular_file()) {
      ret.push_back(entry.path());
    }
  }

  return ret;
}

int main(int argc, char** argv)
{
  testing::InitGoogleTest(&argc, argv);
  auto values_to_test = get_all_tests();
  register_tests(values_to_test);
  return RUN_ALL_TESTS();
}

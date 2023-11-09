#include <algorithm>
#include <deque>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <optional>
#include <regex>
#include <sstream>
#include <string>

#include <fmt/format.h>
#include <gtest/gtest.h>

#include "vm.h"

using namespace std;

namespace
{
constexpr auto expectedOutputPattern = ("// expect: ?(.*)");
constexpr auto expectedErrorPattern = ("// (Error.*)");
constexpr auto errorLinePattern =
    ("// \\[((java|c) )?line (\\d+)\\] (Error.*)");
constexpr auto expectedRuntimeErrorPattern = ("// expect runtime error: (.+)");
constexpr auto syntaxErrorPattern = ("\\[line (\\d+)\\] (Error.+)");
// constexpr auto stackTracePattern = ("\\[line (\\d+)\\]");
// constexpr auto nonTestPattern = ("// nontest");

deque<string> split_newlines(const string& output)
{
  deque<string> res;
  stringstream stream(output);

  string to;
  while (getline(stream, to, '\n')) {
    res.push_back(to);
  }

  return res;
}

bool contains(const deque<string>& haystack, const string& needle)
{
  auto it = find(cbegin(haystack), cend(haystack), needle);

  if (it == cend(haystack)) {
    return false;
  }

  return true;
}

deque<string> difference(const deque<string>& lhs, const deque<string>& rhs)
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

optional<string> match_expected_output(const string& line)
{
  static const regex rgx {expectedOutputPattern};
  smatch match;

  if (regex_search(line, match, rgx)) {
    return match[1];
  }

  return nullopt;
}

optional<string> match_runtime_error(const string& line)
{
  static const regex rgx {expectedRuntimeErrorPattern};
  smatch match;

  if (regex_search(line, match, rgx)) {
    return {match[1]};
  }

  return nullopt;
}

optional<string> match_error_pattern(const string& line)
{
  static const regex rgx {expectedErrorPattern};
  smatch match;

  if (regex_search(line, match, rgx)) {
    return string {match[1]};
  }

  return nullopt;
}

optional<string> match_error_line_pattern(const string& line)
{
  static const regex rgx {errorLinePattern};
  smatch match;

  if (regex_search(line, match, rgx)) {
    return fmt::format("[line {}] {}", string {match[3]}, string {match[4]});
  }

  return nullopt;
}

ExpectedTestResult parse_expected_output_from_lox_source(
    const string& lox_source)
{
  ExpectedTestResult result;

  const auto lines = split_newlines(lox_source);

  for (size_t linenr = 1; linenr <= lines.size(); linenr++) {
    const auto line = lines.at(linenr - 1);

    // check line for output
    if (const auto output = match_expected_output(line)) {
      result.expected_output.push_back(pair {linenr, *output});
      continue;
    }

    // check line for errors
    if (const auto compile_error = match_error_pattern(line)) {
      result.expected_errors.push_back(
          fmt::format("[line {}] {}", linenr, *compile_error));

      continue;
    }

    if (const auto match = match_error_line_pattern(line)) {
      result.expected_errors.push_back(*match);
      continue;
    }

    if (const auto runtime_error = match_runtime_error(line)) {
      EXPECT_FALSE(result.expected_runtime_error.has_value());
      result.expected_runtime_error = runtime_error;
      result.expected_runtime_error_line = linenr;
    }
  }

  return result;
}

void validate_runtime_error(const RawTestOutput& output,
                            const ExpectedTestResult& expected)
{
  const auto errorlines = output.stderrOutput;

  ASSERT_TRUE(expected.expected_runtime_error.has_value());

  const auto expected_runtime_error = *expected.expected_runtime_error;

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

void validate_compile_error(const RawTestOutput& output,
                            const ExpectedTestResult& expected)
{
  // Validate that every compile error was expected.
  deque<string> found_errors;
  int unexpected_count = 0;

  for (const string& line : output.stderrOutput) {
    const regex rgx {syntaxErrorPattern};
    smatch match;

    if (regex_search(line, match, rgx)) {
      const string lineNr = match[1];
      const string what = match[2];

      const auto error = fmt::format("[line {}] {}", lineNr, what);

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
  const auto diff = difference(expected.expected_errors, found_errors);
  for (const auto& error : diff) {
    ADD_FAILURE() << fmt::format("Missing expected error: {}", error);
  }
}

void validate_output(const RawTestOutput& output,
                     const ExpectedTestResult& expected)
{
  auto output_lines = output.stdoutOutput;

  if (!output_lines.empty() && output_lines.back() == "") {
    // remove trailing empty line
    output_lines.pop_back();
  }

  auto index = 0u;

  for (; index < output_lines.size(); index++) {
    const string line = output_lines[index];

    ASSERT_LT(index, expected.expected_output.size());

    const auto exp = expected.expected_output[index];

    EXPECT_EQ(exp.second, line);
  }

  while (index < expected.expected_output.size()) {
    const auto exp = expected.expected_output[index];

    ADD_FAILURE() << fmt::format(
        "Missing expected output '{}' on line {}", exp.second, exp.first);
    index++;
  }
}

void check_results(const ExpectedTestResult& expected,
                   const RawTestOutput& output)
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

void run(const string& source)
{
  const auto expected_output = parse_expected_output_from_lox_source(source);

  VM vm;

  InterpretResult interpret_result;
  stringstream stderrstream, stdoutstream;

  {
    // capture output of vm.interpret

    const auto oldstdout = cout.rdbuf();
    const auto oldstderr = cerr.rdbuf();

    cout.rdbuf(stdoutstream.rdbuf());
    cerr.rdbuf(stderrstream.rdbuf());

    interpret_result = vm.interpret(source);

    cout.rdbuf(oldstdout);
    cerr.rdbuf(oldstderr);
  }

  check_results(expected_output,
                RawTestOutput {interpret_result,
                               split_newlines(stdoutstream.str()),
                               split_newlines(stderrstream.str())});
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

  inline optional<string> read_file(const string& path)
  {
    constexpr auto read_size = 4096u;
    auto stream = ifstream(path.data());
    stream.exceptions(ios_base::badbit);

    if (!stream) {
      return nullopt;
    }

    auto out = string();
    auto buf = string(read_size, '\0');
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

    const auto source = read_file(m_file_path);

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

TestData get_test_names(string path)
{
  // "test/this/this_in_method.lox" -> Fixture: this; Test: this_in_method

  // at most 2 slashes in tests
  assert(count(cbegin(path), cend(path), '/') == 2);

  {
    const auto prefix = string {"test/"};
    const auto idx = path.find(prefix);
    assert(idx != string::npos);

    // "this/this_in_method.lox"
    path.erase(idx, prefix.size());
  }

  {
    const auto suffix = string {".lox"};
    const auto idx = path.find(suffix);
    assert(idx != string::npos);

    // "this/this_in_method"
    path.erase(idx, suffix.size());
  }

  {
    // number of slashes should now be 1
    // e.g "this/this_in_method"

    assert(count(cbegin(path), cend(path), '/') == 1);
  }

  const auto idx = path.find("/");
  assert(idx != string::npos);

  string first = path.substr(0, idx);
  string second = path.substr(idx + 1, path.size() - idx - 1);

  return TestData {first, second};
}

void register_tests(const vector<string>& paths)
{
  for (const auto& path : paths) {
    const auto td = get_test_names(path);

    testing::RegisterTest(
        td.test_suite.c_str(),
        td.test_name.c_str(),
        nullptr,
        nullptr,
        path.c_str(),
        __LINE__,
        // Important to use the fixture type as the return type here.
        [=]() -> LoxTextFixture* { return new LoxTest(path); });
  }
}

vector<string> get_all_tests()
{
  vector<string> ret;

  for (const auto& entry : filesystem::recursive_directory_iterator("test")) {
    const auto str = entry.path().string();

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

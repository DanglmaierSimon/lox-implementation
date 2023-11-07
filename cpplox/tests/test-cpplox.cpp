#include <algorithm>
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

#include <fmt/core.h>
#include <fmt/format.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "vm.h"

using namespace std;

struct RawTestOutput
{
  InterpretResult result;
  deque<string> stdoutOutput;
  deque<string> stderrOutput;
};

struct ExpectedTestResult
{
  string file_name;

  deque<string> expected_output;

  // compiler errors
  deque<string> expected_errors;

  optional<string> expected_runtime_error;

  int expected_runtime_error_line;

  int expected_exit_code;
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

deque<string> stripDebugOutput(deque<std::string> in)
{
  deque<string> res;

  for (auto str : in) {
    if (!(str.rfind("DBG:", 0) == 0)) {
      res.push_back(str);
    }
  }

  return res;
}

optional<string> parse_expected_output(string line)
{
  regex rgx {"// *expect: *(.*)$"};
  smatch match;

  if (regex_search(line.cbegin(), line.cend(), match, rgx)) {
    return match[1];
  }

  return nullopt;
}

optional<string> parse_runtime_error(string line)
{
  // expect runtime error: Undefined variable 'unknown'.

  std::regex rgx {"// *expect runtime error: *(.*)"};
  std::smatch match;

  if (std::regex_search(line.cbegin(), line.cend(), match, rgx)) {
    return RuntimeError {match[1]};
  }

  return std::nullopt;
}

optional<CompileError> parse_compile_error(string line)
{
  std::regex rgx {"// ?(.*)? (Error.*)"};  // pattern for any compile error
  std::smatch match;

  if (std::regex_search(line.cbegin(), line.cend(), match, rgx)) {
    const string lineNr = match[1];
    const string what = match[2];

    return CompileError {std::atoi(lineNr.c_str()), what};
  }

  return nullopt;
}

ExpectedTestResult parse_expected_output_from_lox_source(string lox_source)
{
  ExpectedTestResult result;

  auto lines = split_newlines(lox_source);

  bool found_runtime_error = false;
  bool found_compile_error = false;

  for (size_t linenr = 1; linenr <= lines.size(); linenr++) {
    const auto line = lines.at(linenr - 1);

    // check line for output
    if (auto output = parse_expected_output(line)) {
      EXPECT_FALSE(found_compile_error)
          << "cannot parse output after already having found a compiler or "
             "runtime error";
      EXPECT_FALSE(found_runtime_error)
          << "cannot parse output after already having found a compiler or "
             "runtime error";

      cout << "found expected output at line " << to_string(linenr) << endl;
      result.output.push_back(*output);
    } else if (auto runtime_error = parse_runtime_error(line)) {
      EXPECT_FALSE(found_compile_error)
          << "only 1 compile or runtime error may be found per test";
      EXPECT_FALSE(found_runtime_error)
          << "only 1 compile or runtime error may be found per test";

      cout << "found expected runtime error at line " << to_string(linenr)
           << endl;
      result.rt_error = runtime_error;
      found_runtime_error = true;
    } else if (auto compile_error = parse_compile_error(line)) {
      EXPECT_FALSE(found_compile_error)
          << "only 1 compile or runtime error may be found per test";
      EXPECT_FALSE(found_runtime_error)
          << "only 1 compile or runtime error may be found per test";

      cout << "found expected compile time error at line " << to_string(linenr)
           << endl;
      result.ct_error = compile_error;
      found_compile_error = true;
    } else {
      if (line.find("Error") != string::npos) {
        cout << "found line with error string, but not caught in above "
                "checks!"
             << endl;
        cout << "line: " << line << endl
             << "line nr: " << to_string(linenr) << endl;
      }
    }
  }

  return result;
}

optional<string> parse_actual_output(string line)
{
  if (line.empty()) {
    return nullopt;
  }

  return line;
}

optional<RuntimeError> parse_actual_runtime_error(string prev_line, string line)
{
  std::regex rgx {"line (.*)] in (.*)"};
  std::smatch match;
  if (std::regex_search(line.cbegin(), line.cend(), match, rgx)) {
    const string linenr = match[1];
    const string in = match[2];

    return RuntimeError {prev_line};
  }

  return nullopt;
}

optional<CompileError> parse_actual_compile_error(string line)
{
  std::regex rgx {"(.*)? ?(Error.*)"};
  std::smatch match;
  if (std::regex_search(line.cbegin(), line.cend(), match, rgx)) {
    const string lineNr = match[1];
    const string what = match[2];

    return CompileError {atoi(lineNr.c_str()), what};
  }

  return nullopt;
}

void validate_runtime_error() {}

void validate_compile_error() {}

void validate_exit_code() {}

void validate_output() {}

void check_results(ExpectedTestResult expected,
                   RawTestOutput actual,
                   string file_name)
{
  // runtime and compile time error may not happen at the same time
  ASSERT_FALSE(expected.rt_error.has_value() && expected.ct_error.has_value());

  if (expected.rt_error.has_value()) {
    validate_runtime_error();
  } else if (expected.ct_error.has_value()) {
    validate_compile_error();
  }

  validate_exit_code();
  validate_output();

  while (true) {
    if (actual_deque.empty() || expected_deque.empty()) {
      break;
    }

    const auto act = actual_deque.front();
    const auto exp = expected_deque.front();

    actual_deque.pop_front();
    expected_deque.pop_front();

    EXPECT_EQ(act, exp);
  }

  if ((actual_deque.empty() && expected_deque.empty())) {
    return;
  }

  // drain all the non-received expected values
  while (true) {
    if (expected_deque.empty()) {
      break;
    }

    const auto exp = expected_deque.front();

    expected_deque.pop_front();

    ADD_FAILURE() << "Never received expected output " << exp.to_string();
  }

  while (true) {
    if (actual_deque.empty()) {
      break;
    }

    const auto act = actual_deque.front();

    actual_deque.pop_front();

    if (act.type == OutputType::output
        && file_name.find("benchmark") != string::npos)
    {
      // its a benchmark -> ignore unexpected output
      continue;
    }

    ADD_FAILURE() << "Received unexpected output " << act.to_string();
  }

  cout << "=========================================" << endl;
  cout << "STDOUT: " << endl << out << endl;
  cout << "=========================================" << endl;
  cout << "STDERR: " << endl << err << endl;
  cout << "=========================================" << endl;
}

void check_for_compile_and_runtime_error(vector<TestOutput> expected)
{
  const bool compile_errors =
      count_if(cbegin(expected), cend(expected), [](TestOutput out) {
        return out.type == OutputType::compile_error;
      });

  const bool has_compile_error = compile_errors > 0;

  const bool runtime_errors =
      count_if(cbegin(expected), cend(expected), [](TestOutput out) {
        return out.type == OutputType::runtime_error;
      });

  const bool has_runtime_error = runtime_errors > 0;

  ASSERT_TRUE(runtime_errors == 0 || runtime_errors == 1);

  ASSERT_FALSE(has_compile_error && has_runtime_error)
      << "runtime error and compile time error may not exist at the same "
         "time";
}

void run(string file_name, string source)
{
  (void)file_name;
  const auto expected_output = parse_expected_output_from_lox_source(source);

  check_for_compile_and_runtime_error(expected_output);

  const auto res = run_impl(source);

  const auto actual_results =
      parse_actual_results(res.stderrOutput, res.stdoutOutput);

  check_results(expected_output,
                actual_results,
                res.stderrOutput,
                res.stdoutOutput,
                file_name);
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

    run(m_file_path, *source);
  }

private:
  string m_file_path;
};

void register_tests(const vector<string>& paths)
{
  for (auto path : paths) {
    testing::RegisterTest(
        "MyFixture",
        ("Test" + (path)).c_str(),
        nullptr,
        (path).c_str(),
        __FILE__,
        __LINE__,
        // Important to use the fixture type as the return type here.
        [=]() -> LoxTextFixture* { return new LoxTest(path); });
  }
}

vector<string> get_all_tests()
{
  vector<string> ret;

  auto current = "test";

  for (auto entry : filesystem::recursive_directory_iterator(current)) {
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

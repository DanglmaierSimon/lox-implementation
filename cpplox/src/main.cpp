#include <fstream>
#include <iostream>

#include <sysexits.h>

#include "vm.h"

std::string readFile(const char* path)
{
  std::ifstream file(path);

  if (!file.is_open()) {
    std::cerr << "Could not open file \"" << path << "\".\n";
    exit(EX_NOINPUT);
  }

  return std::string((std::istreambuf_iterator<char>(file)),
                     std::istreambuf_iterator<char>());
}

static void repl()
{
  VM vm;

  while (true) {
    std::cout << "> ";
    std::string line;
    if (!std::getline(std::cin, line)) {
      std::cout << "\n";
      break;
    }

    vm.interpret(line);
  }
}

static void runFile(const char* path)
{
  VM vm;
  const std::string source = readFile(path);
  InterpretResult result = vm.interpret(source);

  if (result == InterpretResult::COMPILE_ERROR) {
    exit(EX_DATAERR);
  }

  if (result == InterpretResult::RUNTIME_ERROR) {
    exit(EX_SOFTWARE);
  }
}

int main(int argc, const char* argv[])
{
  if (argc == 1) {
    repl();
  } else if (argc == 2) {
    runFile(argv[1]);
  } else {
    std::cerr << "Usage: cpplox [path]\n";
    exit(EX_USAGE);
  }

  return 0;
}
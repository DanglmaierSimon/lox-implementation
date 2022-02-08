#include <iostream>

#include <stdio.h>
#include <sysexits.h>

#include "lox/vm.h"

static char* readFile(const char* path)
{
  FILE* file = fopen(path, "rb");

  if (file == nullptr) {
    std::cerr << "Could not open file \"" << path << "\".\n";
    exit(EX_NOINPUT);
  }

  fseek(file, 0L, SEEK_END);
  size_t fileSize = ftell(file);
  rewind(file);

  char* buffer = (char*)malloc(fileSize + 1);

  if (buffer == nullptr) {
    std::cerr << "Not enough memory to read \"" << path << "\".\n";
    exit(EX_OSERR);
  }

  size_t bytesRead = fread(buffer, sizeof(char), fileSize, file);

  if (bytesRead < fileSize) {
    std::cerr << "Could not read file \"" << path << "\".\n";
    exit(EX_NOINPUT);
  }

  buffer[bytesRead] = '\0';

  fclose(file);
  return buffer;
}

static void repl()
{
  VM vm;

  char line[1024];
  while (true) {
    std::cout << "> ";
    if (!fgets(line, sizeof(line), stdin)) {
      std::cout << "\n";
      break;
    }

    vm.interpret(line);
  }
}

static void runFile(const char* path)
{
  VM vm;
  char* source = readFile(path);
  InterpretResult result = vm.interpret(source);
  free(source);

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
    std::cerr << "Usage: clox [path]\n";
    exit(EX_USAGE);
  }

  return 0;
}
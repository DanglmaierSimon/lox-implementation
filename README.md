# lox-implementation
My implementation of the Lox language described in the excellent book "Crafting Interpreters" by Robert Nystrom. Info about the book can be found at https://craftinginterpreters.com/

The file book-challenges.md contains some of the challenges from the book copy-pasted into a compact markdown file.

## Folders

### jlox

Contains the Java tree walking interpreter from the first part of the book.

### cpplox

Contains the (mostly) C implementation of the bytecode interpreter of the second half of the book. The finished C implementation exists up until commit f12d9175f1f615d7b9fae959c615e114faa3e768, from then the effort of porting the compiler to c++, with classes, templates and all that good stuff has started and will replace the C implementation. Additionally i intend to hack around on the language to implement some of the challenges described in the book and also add an additional feature here or there (arrays for example, maybe imports if i feel very brave).

I also want to add some actual unittests to learn GoogleTest and Docker. Some efforts have been started, by converting some of the official testsuite into cpp tests automatically via a script.
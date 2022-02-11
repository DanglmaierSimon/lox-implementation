# Challenges 

## Introduction


## 1 Introduction

1.  There are at least six domain-specific languages used in the [little system
    I cobbled together][repo] to write and publish this book. What are they?

1.  Get a "Hello, world!" program written and running in Java. Set up whatever
    makefiles or IDE projects you need to get it working. If you have a
    debugger, get comfortable with it and step through your program as it runs.

1.  Do the same thing for C. To get some practice with pointers, define a
    [doubly linked list][] of heap-allocated strings. Write functions to insert,
    find, and delete items from it. Test them.

[repo]: https://github.com/munificent/craftinginterpreters
[doubly linked list]: https://en.wikipedia.org/wiki/Doubly_linked_list


## 2 A Map of the teritory

1. Pick an open source implementation of a language you like. Download the
   source code and poke around in it. Try to find the code that implements the
   scanner and parser. Are they handwritten, or generated using tools like
   Lex and Yacc? (`.l` or `.y` files usually imply the latter.)

1. Just-in-time compilation tends to be the fastest way to implement dynamically
   typed languages, but not all of them use it. What reasons are there to *not*
   JIT?

1. Most Lisp implementations that compile to C also contain an interpreter that
   lets them execute Lisp code on the fly as well. Why?

## 3 The Lox Language

1. Write some sample Lox programs and run them (you can use the implementations
   of Lox in [my repository][repo]). Try to come up with edge case behavior I
   didn't specify here. Does it do what you expect? Why or why not?

2. This informal introduction leaves a *lot* unspecified. List several open
   questions you have about the language's syntax and semantics. What do you
   think the answers should be?

3. Lox is a pretty tiny language. What features do you think it is missing that
   would make it annoying to use for real programs? (Aside from the standard
   library, of course.)

## 4 Scanning

1.  The lexical grammars of Python and Haskell are not *regular*. What does that
    mean, and why aren't they?

1.  Aside from separating tokens -- distinguishing `print foo` from `printfoo`
    -- spaces aren't used for much in most languages. However, in a couple of
    dark corners, a space *does* affect how code is parsed in CoffeeScript,
    Ruby, and the C preprocessor. Where and what effect does it have in each of
    those languages?

1.  Our scanner here, like most, discards comments and whitespace since those
    aren't needed by the parser. Why might you want to write a scanner that does
    *not* discard those? What would it be useful for?

1.  Add support to Lox's scanner for C-style `/* ... */` block comments. Make
    sure to handle newlines in them. Consider allowing them to nest. Is adding
    support for nesting more work than you expected? Why?

## 5 Representing Code

1.  Earlier, I said that the `|`, `*`, and `+` forms we added to our grammar
    metasyntax were just syntactic sugar. Take this grammar:

    ```ebnf
    expr → expr ( "(" ( expr ( "," expr )* )? ")" | "." IDENTIFIER )+
         | IDENTIFIER
         | NUMBER
    ```

    Produce a grammar that matches the same language but does not use any of
    that notational sugar.

    *Bonus:* What kind of expression does this bit of grammar encode?

1.  The Visitor pattern lets you emulate the functional style in an
    object-oriented language. Devise a complementary pattern for a functional
    language. It should let you bundle all of the operations on one type
    together and let you define new types easily.

    (SML or Haskell would be ideal for this exercise, but Scheme or another Lisp
    works as well.)

1.  In [reverse Polish notation][rpn] (RPN), the operands to an arithmetic
    operator are both placed before the operator, so `1 + 2` becomes `1 2 +`.
    Evaluation proceeds from left to right. Numbers are pushed onto an implicit
    stack. An arithmetic operator pops the top two numbers, performs the
    operation, and pushes the result. Thus, this:

    ```lox
    (1 + 2) * (4 - 3)
    ```

    in RPN becomes:

    ```lox
    1 2 + 4 3 - *
    ```

    Define a visitor class for our syntax tree classes that takes an expression,
    converts it to RPN, and returns the resulting string.

[rpn]: https://en.wikipedia.org/wiki/Reverse_Polish_notation

## 6 Parsing Expressions

1.  In C, a block is a statement form that allows you to pack a series of
    statements where a single one is expected. The [comma operator][] is an
    analogous syntax for expressions. A comma-separated series of expressions
    can be given where a single expression is expected (except inside a function
    call's argument list). At runtime, the comma operator evaluates the left
    operand and discards the result. Then it evaluates and returns the right
    operand.

    Add support for comma expressions. Give them the same precedence and
    associativity as in C. Write the grammar, and then implement the necessary
    parsing code.

2.  Likewise, add support for the C-style conditional or "ternary" operator
    `?:`. What precedence level is allowed between the `?` and `:`? Is the whole
    operator left-associative or right-associative?

3.  Add error productions to handle each binary operator appearing without a
    left-hand operand. In other words, detect a binary operator appearing at the
    beginning of an expression. Report that as an error, but also parse and
    discard a right-hand operand with the appropriate precedence.

[comma operator]: https://en.wikipedia.org/wiki/Comma_operator


## 7 Evaluating Expressions


1.  Allowing comparisons on types other than numbers could be useful. The
    operators might have a reasonable interpretation for strings. Even
    comparisons among mixed types, like `3 < "pancake"` could be handy to enable
    things like ordered collections of heterogeneous types. Or it could simply
    lead to bugs and confusion.

    Would you extend Lox to support comparing other types? If so, which pairs of
    types do you allow and how do you define their ordering? Justify your
    choices and compare them to other languages.

2.  Many languages define `+` such that if *either* operand is a string, the
    other is converted to a string and the results are then concatenated. For
    example, `"scone" + 4` would yield `scone4`. Extend the code in
    `visitBinaryExpr()` to support that.

3.  What happens right now if you divide a number by zero? What do you think
    should happen? Justify your choice. How do other languages you know handle
    division by zero, and why do they make the choices they do?

    Change the implementation in `visitBinaryExpr()` to detect and report a
    runtime error for this case.

## 8 Statements and State

1.  The REPL no longer supports entering a single expression and automatically
    printing its result value. That's a drag. Add support to the REPL to let
    users type in both statements and expressions. If they enter a statement,
    execute it. If they enter an expression, evaluate it and display the result
    value.

2.  Maybe you want Lox to be a little more explicit about variable
    initialization. Instead of implicitly initializing variables to `nil`, make
    it a runtime error to access a variable that has not been initialized or
    assigned to, as in:

    ```lox
    // No initializers.
    var a;
    var b;

    a = "assigned";
    print a; // OK, was assigned first.

    print b; // Error!
    ```

3.  What does the following program do?

    ```lox
    var a = 1;
    {
      var a = a + 2;
      print a;
    }
    ```

    What did you *expect* it to do? Is it what you think it should do? What
    does analogous code in other languages you are familiar with do? What do
    you think users will expect this to do?

## 9 Control Flow

1.  A few chapters from now, when Lox supports first-class functions and dynamic
    dispatch, we technically won't *need* branching statements built into the
    language. Show how conditional execution can be implemented in terms of
    those. Name a language that uses this technique for its control flow.

2.  Likewise, looping can be implemented using those same tools, provided our
    interpreter supports an important optimization. What is it, and why is it
    necessary? Name a language that uses this technique for iteration.

3.  Unlike Lox, most other C-style languages also support `break` and `continue`
    statements inside loops. Add support for `break` statements.

    The syntax is a `break` keyword followed by a semicolon. It should be a
    syntax error to have a `break` statement appear outside of any enclosing
    loop. At runtime, a `break` statement causes execution to jump to the end of
    the nearest enclosing loop and proceeds from there. Note that the `break`
    may be nested inside other blocks and `if` statements that also need to be
    exited.


## 10 Functions

1.  Our interpreter carefully checks that the number of arguments passed to a
    function matches the number of parameters it expects. Since this check is
    done at runtime on every call, it has a performance cost. Smalltalk
    implementations don't have that problem. Why not?

1.  Lox's function declaration syntax performs two independent operations. It
    creates a function and also binds it to a name. This improves usability for
    the common case where you do want to associate a name with the function.
    But in functional-styled code, you often want to create a function to
    immediately pass it to some other function or return it. In that case, it
    doesn't need a name.

    Languages that encourage a functional style usually support **anonymous
    functions** or **lambdas** -- an expression syntax that creates a function
    without binding it to a name. Add anonymous function syntax to Lox so that
    this works:

    ```lox
    fun thrice(fn) {
      for (var i = 1; i <= 3; i = i + 1) {
        fn(i);
      }
    }

    thrice(fun (a) {
      print a;
    });
    // "1".
    // "2".
    // "3".
    ```

    How do you handle the tricky case of an anonymous function expression
    occurring in an expression statement:

    ```lox
    fun () {};
    ```

1.  Is this program valid?

    ```lox
    fun scope(a) {
      var a = "local";
    }
    ```

    In other words, are a function's parameters in the *same* scope as its local
    variables, or in an outer scope? What does Lox do? What about other
    languages you are familiar with? What do you think a language *should* do?


## 11 Resolving and Binding

1.  Why is it safe to eagerly define the variable bound to a function's name
    when other variables must wait until after they are initialized before they
    can be used?

1.  How do other languages you know handle local variables that refer to the
    same name in their initializer, like:

    ```lox
    var a = "outer";
    {
      var a = a;
    }
    ```

    Is it a runtime error? Compile error? Allowed? Do they treat global
    variables differently? Do you agree with their choices? Justify your answer.

1.  Extend the resolver to report an error if a local variable is never used.

1.  Our resolver calculates *which* environment the variable is found in, but
    it's still looked up by name in that map. A more efficient environment
    representation would store local variables in an array and look them up by
    index.

    Extend the resolver to associate a unique index for each local variable
    declared in a scope. When resolving a variable access, look up both the
    scope the variable is in and its index and store that. In the interpreter,
    use that to quickly access a variable by its index instead of using a map.

## 12 Classes

1.  We have methods on instances, but there is no way to define "static" methods
    that can be called directly on the class object itself. Add support for
    them. Use a `class` keyword preceding the method to indicate a static method
    that hangs off the class object.

    ```lox
    class Math {
      class square(n) {
        return n * n;
      }
    }

    print Math.square(3); // Prints "9".
    ```

    You can solve this however you like, but the "[metaclasses][]" used by
    Smalltalk and Ruby are a particularly elegant approach. *Hint: Make LoxClass
    extend LoxInstance and go from there.*

2.  Most modern languages support "getters" and "setters" -- members on a class
    that look like field reads and writes but that actually execute user-defined
    code. Extend Lox to support getter methods. These are declared without a
    parameter list. The body of the getter is executed when a property with that
    name is accessed.

    ```lox
    class Circle {
      init(radius) {
        this.radius = radius;
      }

      area {
        return 3.141592653 * this.radius * this.radius;
      }
    }

    var circle = Circle(4);
    print circle.area; // Prints roughly "50.2655".
    ```

3.  Python and JavaScript allow you to freely access an object's fields from
    outside of its own methods. Ruby and Smalltalk encapsulate instance state.
    Only methods on the class can access the raw fields, and it is up to the
    class to decide which state is exposed. Most statically typed languages
    offer modifiers like `private` and `public` to control which parts of a
    class are externally accessible on a per-member basis.

    What are the trade-offs between these approaches and why might a language
    prefer one or the other?

[metaclasses]: https://en.wikipedia.org/wiki/Metaclass

## 13 Inheritance

1.  Lox supports only *single inheritance* -- a class may have a single
    superclass and that's the only way to reuse methods across classes. Other
    languages have explored a variety of ways to more freely reuse and share
    capabilities across classes: mixins, traits, multiple inheritance, virtual
    inheritance, extension methods, etc.

    If you were to add some feature along these lines to Lox, which would you
    pick and why? If you're feeling courageous (and you should be at this
    point), go ahead and add it.

1.  In Lox, as in most other object-oriented languages, when looking up a
    method, we start at the bottom of the class hierarchy and work our way up --
    a subclass's method is preferred over a superclass's. In order to get to the
    superclass method from within an overriding method, you use `super`.

    The language [BETA][] takes the [opposite approach][inner]. When you call a
    method, it starts at the *top* of the class hierarchy and works *down*. A
    superclass method wins over a subclass method. In order to get to the
    subclass method, the superclass method can call `inner`, which is sort of
    like the inverse of `super`. It chains to the next method down the
    hierarchy.

    The superclass method controls when and where the subclass is allowed to
    refine its behavior. If the superclass method doesn't call `inner` at all,
    then the subclass has no way of overriding or modifying the superclass's
    behavior.

    Take out Lox's current overriding and `super` behavior and replace it with
    BETA's semantics. In short:

    *   When calling a method on a class, prefer the method *highest* on the
        class's inheritance chain.

    *   Inside the body of a method, a call to `inner` looks for a method with
        the same name in the nearest subclass along the inheritance chain
        between the class containing the `inner` and the class of `this`. If
        there is no matching method, the `inner` call does nothing.

    For example:

    ```lox
    class Doughnut {
      cook() {
        print "Fry until golden brown.";
        inner();
        print "Place in a nice box.";
      }
    }

    class BostonCream < Doughnut {
      cook() {
        print "Pipe full of custard and coat with chocolate.";
      }
    }

    BostonCream().cook();
    ```

    This should print:

    ```text
    Fry until golden brown.
    Pipe full of custard and coat with chocolate.
    Place in a nice box.
    ```

1.  In the chapter where I introduced Lox, [I challenged you][challenge] to
    come up with a couple of features you think the language is missing. Now
    that you know how to build an interpreter, implement one of those features.

[challenge]: the-lox-language.html#challenges
[inner]: http://journal.stuffwithstuff.com/2012/12/19/the-impoliteness-of-overriding-methods/
[beta]: https://beta.cs.au.dk/



## 14 Chunks of bytecode



1. Our encoding of line information is hilariously wasteful of memory. Given that a series of instructions often correspond to the same source line, a natural solution is something akin to run-length encoding of the line numbers.

Devise an encoding that compresses the line information for a series of instructions on the same line. Change writeChunk() to write this compressed form, and implement a getLine() function that, given the index of an instruction, determines the line where the instruction occurs.

Hint: It’s not necessary for getLine() to be particularly efficient. Since it is called only when a runtime error occurs, it is well off the critical path where performance matters.

2. Because OP_CONSTANT uses only a single byte for its operand, a chunk may only contain up to 256 different constants. That’s small enough that people writing real-world code will hit that limit. We could use two or more bytes to store the operand, but that makes every constant instruction take up more space. Most chunks won’t need that many unique constants, so that wastes space and sacrifices some locality in the common case to support the rare case.

To balance those two competing aims, many instruction sets feature multiple instructions that perform the same operation but with operands of different sizes. Leave our existing one-byte OP_CONSTANT instruction alone, and define a second OP_CONSTANT_LONG instruction. It stores the operand as a 24-bit number, which should be plenty.

Implement this function:
```
void writeConstant(Chunk* chunk, Value value, int line) {
  // Implement me...
}
```
It adds value to chunk’s constant array and then writes an appropriate instruction to load the constant. Also add support to the disassembler for OP_CONSTANT_LONG instructions.

Defining two instructions seems to be the best of both worlds. What sacrifices, if any, does it force on us?

3. Our reallocate() function relies on the C standard library for dynamic memory allocation and freeing. malloc() and free() aren’t magic. Find a couple of open source implementations of them and explain how they work. How do they keep track of which bytes are allocated and which are free? What is required to allocate a block of memory? Free it? How do they make that efficient? What do they do about fragmentation?

Hardcore mode: Implement reallocate() without calling realloc(), malloc(), or free(). You are allowed to call malloc() once, at the beginning of the interpreter’s execution, to allocate a single big block of memory, which your reallocate() function has access to. It parcels out blobs of memory from that single region, your own personal heap. It’s your job to define how it does that.


## 15 A Virtual Machine

1. What bytecode instruction sequences would you generate for the following expressions:
```
1 * 2 + 3
1 + 2 * 3
3 - 2 - 1
1 + 2 * 3 - 4 / -5
```
(Remember that Lox does not have a syntax for negative number literals, so the -5 is negating the number 5.)

2. If we really wanted a minimal instruction set, we could eliminate either OP_NEGATE or OP_SUBTRACT. Show the bytecode instruction sequence you would generate for:
```
4 - 3 * -2
```
First, without using OP_NEGATE. Then, without using OP_SUBTRACT.

Given the above, do you think it makes sense to have both instructions? Why or why not? Are there any other redundant instructions you would consider including?

3. Our VM’s stack has a fixed size, and we don’t check if pushing a value overflows it. This means the wrong series of instructions could cause our interpreter to crash or go into undefined behavior. Avoid that by dynamically growing the stack as needed.

What are the costs and benefits of doing so?

4. To interpret OP_NEGATE, we pop the operand, negate the value, and then push the result. That’s a simple implementation, but it increments and decrements stackTop unnecessarily, since the stack ends up the same height in the end. It might be faster to simply negate the value in place on the stack and leave stackTop alone. Try that and see if you can measure a performance difference.

Are there other instructions where you can do a similar optimization?

## 16 Scanning on Demand

1. Many newer languages support string interpolation. Inside a string literal, you have some sort of special delimiters—most commonly ${ at the beginning and } at the end. Between those delimiters, any expression can appear. When the string literal is executed, the inner expression is evaluated, converted to a string, and then merged with the surrounding string literal.

For example, if Lox supported string interpolation, then this ...
```
var drink = "Tea";
var steep = 4;
var cool = 2;
print "${drink} will be ready in ${steep + cool} minutes.";
```
... would print:
```
Tea will be ready in 6 minutes.
```
What token types would you define to implement a scanner for string interpolation? What sequence of tokens would you emit for the above string literal?

What tokens would you emit for:

`"Nested ${"interpolation?! Are you ${"mad?!"}"}"`
Consider looking at other language implementations that support interpolation to see how they handle it.

2. Several languages use angle brackets for generics and also have a >> right shift operator. This led to a classic problem in early versions of C++:
```c++
vector<vector<string>> nestedVectors;
```
This would produce a compile error because the >> was lexed to a single right shift token, not two > tokens. Users were forced to avoid this by putting a space between the closing angle brackets.

Later versions of C++ are smarter and can handle the above code. Java and C# never had the problem. How do those languages specify and implement this?

3. Many languages, especially later in their evolution, define “contextual keywords”. These are identifiers that act like reserved words in some contexts but can be normal user-defined identifiers in others.

For example, await is a keyword inside an async method in C#, but in other methods, you can use await as your own identifier.

Name a few contextual keywords from other languages, and the context where they are meaningful. What are the pros and cons of having contextual keywords? How would you implement them in your language’s front end if you needed to?


## 17 Compiling Expressions

1. To really understand the parser, you need to see how execution threads through the interesting parsing functions—parsePrecedence() and the parser functions stored in the table. Take this (strange) expression:
```
(-1 + 2) * 3 - -4
```
Write a trace of how those functions are called. Show the order they are called, which calls which, and the arguments passed to them.

2. The ParseRule row for TOKEN_MINUS has both prefix and infix function pointers. That’s because - is both a prefix operator (unary negation) and an infix one (subtraction). In the full Lox language, what other tokens can be used in both prefix and infix positions? What about in C or in another language of your choice?

3. You might be wondering about complex “mixfix” expressions that have more than two operands separated by tokens. C’s conditional or “ternary” operator, ?:, is a widely known one. Add support for that operator to the compiler. You don’t have to generate any bytecode, just show how you would hook it up to the parser and handle the operands.

## 18 Types of Values

1. We could reduce our binary operators even further than we did here. Which other instructions can you eliminate, and how would the compiler cope with their absence?

2. Conversely, we can improve the speed of our bytecode VM by adding more specific instructions that correspond to higher-level operations. What instructions would you define to speed up the kind of user code we added support for in this chapter

## 19 Strings

1. Each string requires two separate dynamic allocations—one for the ObjString and a second for the character array. Accessing the characters from a value requires two pointer indirections, which can be bad for performance. A more efficient solution relies on a technique called flexible array members. Use that to store the ObjString and its character array in a single contiguous allocation.
See https://en.wikipedia.org/wiki/Flexible_array_member

2. When we create the ObjString for each string literal, we copy the characters onto the heap. That way, when the string is later freed, we know it is safe to free the characters too.

This is a simpler approach but wastes some memory, which might be a problem on very constrained devices. Instead, we could keep track of which ObjStrings own their character array and which are “constant strings” that just point back to the original source string or some other non-freeable location. Add support for this.

3. If Lox was your language, what would you have it do when a user tries to use + with one string operand and the other some other type? Justify your choice. What do other languages do?

## 20 Hash Tables 

1. In clox, we happen to only need keys that are strings, so the hash table we built is hardcoded for that key type. If we exposed hash tables to Lox users as a first-class collection, it would be useful to support different kinds of keys.

Add support for keys of the other primitive types: numbers, Booleans, and nil. Later, clox will support user-defined classes. If we want to support keys that are instances of those classes, what kind of complexity does that add?

2. Hash tables have a lot of knobs you can tweak that affect their performance. You decide whether to use separate chaining or open addressing. Depending on which fork in that road you take, you can tune how many entries are stored in each node, or the probing strategy you use. You control the hash function, load factor, and growth rate.

All of this variety wasn’t created just to give CS doctoral candidates something to publish theses on: each has its uses in the many varied domains and hardware scenarios where hashing comes into play. Look up a few hash table implementations in different open source systems, research the choices they made, and try to figure out why they did things that way.

3. Benchmarking a hash table is notoriously difficult. A hash table implementation may perform well with some keysets and poorly with others. It may work well at small sizes but degrade as it grows, or vice versa. It may choke when deletions are common, but fly when they aren’t. Creating benchmarks that accurately represent how your users will use the hash table is a challenge.

Write a handful of different benchmark programs to validate our hash table implementation. How does the performance vary between them? Why did you choose the specific test cases you chose?

## 21 Global Variables

1. The compiler adds a global variable’s name to the constant table as a string every time an identifier is encountered. It creates a new constant each time, even if that variable name is already in a previous slot in the constant table. That’s wasteful in cases where the same variable is referenced multiple times by the same function. That, in turn, increases the odds of filling up the constant table and running out of slots since we allow only 256 constants in a single chunk.

Optimize this. How does your optimization affect the performance of the compiler compared to the runtime? Is this the right trade-off?

2. Looking up a global variable by name in a hash table each time it is used is pretty slow, even with a good hash table. Can you come up with a more efficient way to store and access global variables without changing the semantics?


3. When running in the REPL, a user might write a function that references an unknown global variable. Then, in the next line, they declare the variable. Lox should handle this gracefully by not reporting an “unknown variable” compile error when the function is first defined.

But when a user runs a Lox script, the compiler has access to the full text of the entire program before any code is run. Consider this program:

```
fun useVar() {
  print oops;
}

var ooops = "too many o's!";
```

Here, we can tell statically that oops will not be defined because there is no declaration of that global anywhere in the program. Note that useVar() is never called either, so even though the variable isn’t defined, no runtime error will occur because it’s never used either.

We could report mistakes like this as compile errors, at least when running from a script. Do you think we should? Justify your answer. What do other scripting languages you know do?

## 21 Local Variables

1. Our simple local array makes it easy to calculate the stack slot of each local variable. But it means that when the compiler resolves a reference to a variable, we have to do a linear scan through the array.

Come up with something more efficient. Do you think the additional complexity is worth it?

2. How do other languages handle code like this:
```
var a = a;
```
What would you do if it was your language? Why?

3. Many languages make a distinction between variables that can be reassigned and those that can’t. In Java, the final modifier prevents you from assigning to a variable. In JavaScript, a variable declared with let can be assigned, but one declared using const can’t. Swift treats let as single-assignment and uses var for assignable variables. Scala and Kotlin use val and var.

Pick a keyword for a single-assignment variable form to add to Lox. Justify your choice, then implement it. An attempt to assign to a variable declared using your new keyword should cause a compile error.

4. Extend clox to allow more than 256 local variables to be in scope at a time.

## 23 Jumping Back and Forth

1. In addition to if statements, most C-family languages have a multi-way switch statement. Add one to clox. The grammar is:
```
switchStmt     → "switch" "(" expression ")"
                 "{" switchCase* defaultCase? "}" ;
switchCase     → "case" expression ":" statement* ;
defaultCase    → "default" ":" statement* ;
```
To execute a switch statement, first evaluate the parenthesized switch value expression. Then walk the cases. For each case, evaluate its value expression. If the case value is equal to the switch value, execute the statements under the case and then exit the switch statement. Otherwise, try the next case. If no case matches and there is a default clause, execute its statements.

To keep things simpler, we’re omitting fallthrough and break statements. Each case automatically jumps to the end of the switch statement after its statements are done.

2. In jlox, we had a challenge to add support for break statements. This time, let’s do continue:
```
continueStmt   → "continue" ";" ;
```
A continue statement jumps directly to the top of the nearest enclosing loop, skipping the rest of the loop body. Inside a for loop, a continue jumps to the increment clause, if there is one. It’s a compile-time error to have a continue statement not enclosed in a loop.

Make sure to think about scope. What should happen to local variables declared inside the body of the loop or in blocks nested inside the loop when a continue is executed?

3. Control flow constructs have been mostly unchanged since Algol 68. Language evolution since then has focused on making code more declarative and high level, so imperative control flow hasn’t gotten much attention.

For fun, try to invent a useful novel control flow feature for Lox. It can be a refinement of an existing form or something entirely new. In practice, it’s hard to come up with something useful enough at this low expressiveness level to outweigh the cost of forcing a user to learn an unfamiliar notation and behavior, but it’s a good chance to practice your design skills.

## 24 Calls and Functions

1. Reading and writing the ip field is one of the most frequent operations inside the bytecode loop. Right now, we access it through a pointer to the current CallFrame. That requires a pointer indirection which may force the CPU to bypass the cache and hit main memory. That can be a real performance sink.

Ideally, we’d keep the ip in a native CPU register. C doesn’t let us require that without dropping into inline assembly, but we can structure the code to encourage the compiler to make that optimization. If we store the ip directly in a C local variable and mark it register, there’s a good chance the C compiler will accede to our polite request.

This does mean we need to be careful to load and store the local ip back into the correct CallFrame when starting and ending function calls. Implement this optimization. Write a couple of benchmarks and see how it affects the performance. Do you think the extra code complexity is worth it?

2. Native function calls are fast in part because we don’t validate that the call passes as many arguments as the function expects. We really should, or an incorrect call to a native function without enough arguments could cause the function to read uninitialized memory. Add arity checking.

3. Right now, there’s no way for a native function to signal a runtime error. In a real implementation, this is something we’d need to support because native functions live in the statically typed world of C but are called from dynamically typed Lox land. If a user, say, tries to pass a string to sqrt(), that native function needs to report a runtime error.

Extend the native function system to support that. How does this capability affect the performance of native calls?

4. Add some more native functions to do things you find useful. Write some programs using those. What did you add? How do they affect the feel of the language and how practical it is?

## 25 Closures

1. Wrapping every ObjFunction in an ObjClosure introduces a level of indirection that has a performance cost. That cost isn’t necessary for functions that do not close over any variables, but it does let the runtime treat all calls uniformly.

Change clox to only wrap functions in ObjClosures that need upvalues. How does the code complexity and performance compare to always wrapping functions? Take care to benchmark programs that do and do not use closures. How should you weight the importance of each benchmark? If one gets slower and one faster, how do you decide what trade-off to make to choose an implementation strategy?


2. Read the design note below. I’ll wait. Now, how do you think Lox should behave? Change the implementation to create a new variable for each loop iteration. (see https://craftinginterpreters.com/closures.html)

3. A famous koan teaches us that “objects are a poor man’s closure” (and vice versa). Our VM doesn’t support objects yet, but now that we have closures we can approximate them. Using closures, write a Lox program that models two-dimensional vector “objects”. It should:

Define a “constructor” function to create a new vector with the given x and y coordinates.

Provide “methods” to access the x and y coordinates of values returned from that constructor.

Define an addition “method” that adds two vectors and produces a third.

## 26 Garbage Collerction

1. The Obj header struct at the top of each object now has three fields: type, isMarked, and next. How much memory do those take up (on your machine)? Can you come up with something more compact? Is there a runtime cost to doing so?

2. When the sweep phase traverses a live object, it clears the isMarked field to prepare it for the next collection cycle. Can you come up with a more efficient approach?

3. Mark-sweep is only one of a variety of garbage collection algorithms out there. Explore those by replacing or augmenting the current collector with another one. Good candidates to consider are reference counting, Cheney’s algorithm, or the Lisp 2 mark-compact algorithm.

## 27 Classes and Instances

1. Trying to access a non-existent field on an object immediately aborts the entire VM. The user has no way to recover from this runtime error, nor is there any way to see if a field exists before trying to access it. It’s up to the user to ensure on their own that only valid fields are read.

How do other dynamically typed languages handle missing fields? What do you think Lox should do? Implement your solution.

2. Fields are accessed at runtime by their string name. But that name must always appear directly in the source code as an identifier token. A user program cannot imperatively build a string value and then use that as the name of a field. Do you think they should be able to? Devise a language feature that enables that and implement it.

3. Conversely, Lox offers no way to remove a field from an instance. You can set a field’s value to nil, but the entry in the hash table is still there. How do other languages handle this? Choose and implement a strategy for Lox.

4. Because fields are accessed by name at runtime, working with instance state is slow. It’s technically a constant-time operation—thanks, hash tables—but the constant factors are relatively large. This is a major component of why dynamic languages are slower than statically typed ones.

How do sophisticated implementations of dynamically typed languages cope with and optimize this?

## 28 Methods and Initializers

1. The hash table lookup to find a class’s init() method is constant time, but still fairly slow. Implement something faster. Write a benchmark and measure the performance difference.

2. In a dynamically typed language like Lox, a single callsite may invoke a variety of methods on a number of classes throughout a program’s execution. Even so, in practice, most of the time a callsite ends up calling the exact same method on the exact same class for the duration of the run. Most calls are actually not polymorphic even if the language says they can be.

How do advanced language implementations optimize based on that observation?

## 29 Superclasses

1. A tenet of object-oriented programming is that a class should ensure new objects are in a valid state. In Lox, that means defining an initializer that populates the instance’s fields. Inheritance complicates invariants because the instance must be in a valid state according to all of the classes in the object’s inheritance chain.

The easy part is remembering to call super.init() in each subclass’s init() method. The harder part is fields. There is nothing preventing two classes in the inheritance chain from accidentally claiming the same field name. When this happens, they will step on each other’s fields and possibly leave you with an instance in a broken state.

If Lox was your language, how would you address this, if at all? If you would change the language, implement your change.

2. Our copy-down inheritance optimization is valid only because Lox does not permit you to modify a class’s methods after its declaration. This means we don’t have to worry about the copied methods in the subclass getting out of sync with later changes to the superclass.

Other languages, like Ruby, do allow classes to be modified after the fact. How do implementations of languages like that support class modification while keeping method resolution efficient?

3. In the jlox chapter on inheritance, we had a challenge to implement the BETA language’s approach to method overriding. Solve the challenge again, but this time in clox. Here’s the description of the previous challenge:

In Lox, as in most other object-oriented languages, when looking up a method, we start at the bottom of the class hierarchy and work our way up—a subclass’s method is preferred over a superclass’s. In order to get to the superclass method from within an overriding method, you use super.

The language BETA takes the opposite approach. When you call a method, it starts at the top of the class hierarchy and works down. A superclass method wins over a subclass method. In order to get to the subclass method, the superclass method can call inner, which is sort of like the inverse of super. It chains to the next method down the hierarchy.

The superclass method controls when and where the subclass is allowed to refine its behavior. If the superclass method doesn’t call inner at all, then the subclass has no way of overriding or modifying the superclass’s behavior.

Take out Lox’s current overriding and super behavior, and replace it with BETA’s semantics. In short:

When calling a method on a class, the method highest on the class’s inheritance chain takes precedence.

Inside the body of a method, a call to inner looks for a method with the same name in the nearest subclass along the inheritance chain between the class containing the inner and the class of this. If there is no matching method, the inner call does nothing.

For example:
```
class Doughnut {
  cook() {
    print "Fry until golden brown.";
    inner();
    print "Place in a nice box.";
  }
}

class BostonCream < Doughnut {
  cook() {
    print "Pipe full of custard and coat with chocolate.";
  }
}

BostonCream().cook();
```
This should print:
```
Fry until golden brown.
Pipe full of custard and coat with chocolate.
Place in a nice box.
```
Since clox is about not just implementing Lox, but doing so with good performance, this time around try to solve the challenge with an eye towards efficiency.


## 30 Optimization

1. Fire up your profiler, run a couple of benchmarks, and look for other hotspots in the VM. Do you see anything in the runtime that you can improve?


2. Many strings in real-world user programs are small, often only a character or two. This is less of a concern in clox because we intern strings, but most VMs don’t. For those that don’t, heap allocating a tiny character array for each of those little strings and then representing the value as a pointer to that array is wasteful. Often, the pointer is larger than the string’s characters. A classic trick is to have a separate value representation for small strings that stores the characters inline in the value.

Starting from clox’s original tagged union representation, implement that optimization. Write a couple of relevant benchmarks and see if it helps.

3. Reflect back on your experience with this book. What parts of it worked well for you? What didn’t? Was it easier for you to learn bottom-up or top-down? Did the illustrations help or distract? Did the analogies clarify or confuse?

The more you understand your personal learning style, the more effectively you can upload knowledge into your head. You can specifically target material that teaches you the way you learn best.
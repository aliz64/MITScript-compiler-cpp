# Compiler for MITScript Langugage (By Ali Zartash)

The folders contain the following items:

## doc/

This contains the documentation and design decisions for the compiler.

## parser/

This contains the parser for an MITScript source file (written using Flex and Bison parser generator) as well as an Abstract Syntax Tree (AST) representation. It also contains a recursive interpreter which directly runs the AST in C++ without any intermediate representation or compilation. This version of the interpreter is not garbage collected and will leak memory.
Related to Assignments 1 and 2 of the class.

## vm/

This contains the MITScript Virtial Machine. It consists of a compiler to translate MITScript source code into an intermediate bytecode representation. It also contains a recursion-free interpreter for the intermediate bytecode representation. Garbage collection will also be implemented.
Related to Assignment 3 of the class.

## gc/

Contains files to support garbage collection for the VM (Bytecode Interpreter) described above.
The implementation uses the mark-and-sweep garbage collection mechanism.
Related to Assignment 4 of the class.

## tests/

Contains test files and expected output files. Also has some neat scripts that automate running of the tests.


# MiniLang Compiler

A complete implementation of a custom scripting language compiler demonstrating all six phases of compilation from lexical analysis to code execution.

## Project Overview

MiniLang is a minimal integer-only imperative language designed for numerical pattern generation and educational compiler implementation. This project implements a complete compiler workflow including lexical analysis, syntax parsing, semantic checking, optimization, and code generation.

## Language Specification

### Lexical Rules
- Integer literals: sequence of digits (0, 123)
- Identifiers: letter/underscore followed by letters/digits/underscores
- Keywords: print, if, else, while
- Operators: + - * / % == != < > <= >= =
- Delimiters: ( ) { } ;
- Comments: line comments starting with //

### Grammar
program := stmt_list
stmt_list := { statement }
statement := assignment ';' | print_stmt ';' | if_stmt | while_stmt | block
assignment := IDENT '=' expr
print_stmt := 'print' '(' expr ')'
if_stmt := 'if' '(' expr ')' block [ 'else' block ]
while_stmt := 'while' '(' expr ')' block
block := '{' stmt_list '}'
expr := equality
equality := comparison { ( '==' | '!=' ) comparison }
comparison := term { ('<' | '>' | '<=' | '>=') term }
term := factor { ('+' | '-') factor }
factor := unary { ('*' | '/' | '%') unary }
unary := ('+' | '-') unary | primary
primary := INT_LIT | IDENT | '(' expr ')'



### Type System
- Single type: integer (64-bit signed)
- Variables must be assigned before use
- All operators work on integers and yield integers
- Comparison operators return 1 (true) or 0 (false)

## Compilation

```bash
g++ -std=c++17 minilang.cpp -o minilang -O2
g++ -std=c++17 menu.cpp -o menu

# Usage

./minilang
./minilang factorial.minilang
./minilang --spec
./minilang -v factorial.minilang
./menu

Example Programs
Factorial
minilang
n = 5;
res = 1;
i = 1;
while (i <= n) {
    res = res * i;
    i = i + 1;
}
print(res);
Fibonacci
minilang
a = 0;
b = 1;
i = 0;
while (i < 10) {
    t = a + b;
    a = b;
    b = t;
    i = i + 1;
}
print(a);
Compiler Phases
Lexical Analysis: Tokenization and symbol recognition

Syntax Analysis: Recursive descent parsing and AST generation

Semantic Analysis: Type checking and symbol table management

Intermediate Code Generation: Three-address code output

Optimization: Constant folding and expression simplification

Code Execution: Interpreter with environment management

Project Structure
minilang.cpp: Main compiler implementation

menu.cpp: Interactive pattern selector

*.minilang: Example program files

setup.sh: Setup script for examples

Error Handling
Comprehensive error reporting for lexical, syntax, semantic, and runtime errors including undefined variables and division by zero.

Limitations
Integer-only arithmetic

No function definitions

No string or array types

No input capabilities

Basic error recovery

Academic Context
Educational project demonstrating complete compiler construction process covering finite automata, context-free grammars, syntax-directed translation, and code optimization.

Development
Implemented in C++17 with clear phase separation in single-file architecture for educational clarity.

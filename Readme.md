# MiniLang Compiler

A complete implementation of a custom scripting language compiler demonstrating all six phases of compilation—from lexical analysis to code execution.

---

## Project Overview

MiniLang is a minimal integer-only imperative language designed for educational compiler design.  
This project implements the full compilation workflow:

1. Lexical Analysis  
2. Syntax Analysis  
3. Semantic Analysis  
4. Intermediate Code Generation  
5. Optimization  
6. Execution (Interpreter-Based)

MiniLang is excellent for learning how real compilers work—from DFAs and CFGs to ASTs and three-address code.

---

## Language Specification

### **Lexical Rules**

- **Integer literals**: `0`, `123`, `9075`
- **Identifiers**: `[A-Za-z_][A-Za-z0-9_]*`
- **Keywords**: `print`, `if`, `else`, `while`
- **Operators**: `+ - * / % == != < > <= >= =`
- **Delimiters**: `(` `)` `{` `}` `;`
- **Comments**: `// comment until end of line`
- **Whitespace**: ignored except for token separation

---

## Grammar (BNF Form)

program := stmt_list
stmt_list := { statement }

statement := assignment ';'
| print_stmt ';'
| if_stmt
| while_stmt
| block

assignment := IDENT '=' expr
print_stmt := 'print' '(' expr ')'
if_stmt := 'if' '(' expr ')' block [ 'else' block ]
while_stmt := 'while' '(' expr ')' block
block := '{' stmt_list '}'

expr := equality
equality := comparison { ( '==' | '!=' ) comparison }
comparison := term { ( '<' | '>' | '<=' | '>=' ) term }
term := factor { ( '+' | '-' ) factor }
factor := unary { ( '*' | '/' | '%' ) unary }
unary := ( '+' | '-' ) unary | primary
primary := INT_LIT | IDENT | '(' expr ')'


---

## Type System

- Single primitive type: **64-bit signed integer**
- Variables must be defined before use
- All operators return integers
- Comparisons yield **1 for true** and **0 for false**

---

## Compilation

### **Build Commands**

```
g++ -std=c++17 minilang.cpp -o minilang -O2
g++ -std=c++17 menu.cpp -o menu
```

## Usage
```
./minilang
./minilang factorial.minilang
./minilang --spec
./minilang -v fibonacci.minilang
./menu
```

## Compiler Phases
### 1. Lexical Analysis
Tokenization of:
identifiers
numbers
keywords
operators
delimiters

### 2. Syntax Analysis
Recursive descent parser
Abstract Syntax Tree (AST) construction

### 3. Semantic Analysis
Symbol table management
Variable declaration/use validation
Type checking

### 4. Intermediate Code Generation
Three-address code (TAC)
Explicit temporaries for expressions

### 5. Optimization
Constant folding
Expression simplification

### 6. Execution
Stack-based interpreter
Environment table for variable bindings

### Project Structure
minilang.cpp    → Full compiler implementation
menu.cpp        → CLI program to run built-in examples
*.minilang      → Sample programs
setup.sh        → Environment setup script
README.md       → Documentation

## Error Handling

### The compiler reports:
Lexical errors
Syntax errors
Semantic errors (undefined variables, uninitialized usage)
Runtime errors (division by zero)
Errors include line numbers for debugging.

## Limitations
Integer-only language
No functions
No arrays or strings
No input support
Basic error recovery
Academic Context

## This compiler demonstrates:
DFAs for lexical analysis
Context-free grammar design
Recursive descent parsing
AST-based semantic analysis
Compiler optimizations
Intermediate code & program execution
It is suitable for Compiler Construction, Programming Languages, and Systems Programming courses.

## Development Notes
Implemented in C++17
Clean single-file architecture for learning clarity

## Easy to extend for:
functions
type system
arrays
advanced optimizations

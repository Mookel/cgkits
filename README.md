# cgkits
## Introduction

Three compiler-generation tools originally written by Allen I. Holub are refactored: 

- LeX: a tool like flex/lex, used for lexical analysis.
- LLAMA: a LL(1) parser generation tool.
- OCCS: a LALR(1) parser generation tool just like yacc/bison, used for syntax analysis.

They were originally written in old C style by Allen in 1992 based on DOS and Unix platform.These tools are fantasty, but unfortunately the source code is very hard going and tools are not fit in modern platforms. To learn how this tools are built and make them available on modern platforms, I decided to refactor all of them.

## Refactoring

I almost rewrite all source code using C and make it executable on OS X and Ubuntu platform. Compared to original one, there are serval improvements :

- Interface and variable names are renamed to make them more clear and readable.
- Interfaces with similar function are abstracted into common modules, which are easier maintained.
- Get rid of usage of global variables, reducing coupling of different module.
- Add gc support. The simplest malloc/free interfaces were used by Allen for his memory management, leading to many memory leak problems in his code. To solve this, I add gc support.
- The tools are ported to OS X and Ubuntu/other Linux platform.
- Fix some bugs of original code.

## Contact me

If you have any problem or report some bugs, just feel free to email me:

Email: ltp0709@sina.com 


code in this repository should be written in C++17.

follow this guideline for code contribution:

## code standard
- code indentation is 2 spaces, never use a tab character
- namespace does not contribute to indentation
- opening brace should be on the same line as its block signature
- do not use pragma once, use the classic macro approach to protect header inclusion recursion
- when using std library, define namespace alias s to refer to std and use the alias instead
- when including headers from source code, always use angle brackets instead of double quotes, configure cmake accordingly
- switch statement should use break case style by placing a break before the case, even for the first case
- prefer adding optional comma character when comma is optional in C++ syntax, such as comma in enum
- prefer using struct over class, except for enum
- prefer using typename over class in template parameter
- enum and enum class definition should always be explicit on storage type, which is usually int
- use and, or, not, xor instead of && || ! ^
- prefer using switch statement in situation when a single decision point have multiple logic branches
- class member variables should be private or public, all class member functions should be either public or protected, never private
- prefer composition over inheritance
- avoid using inheritance on small functions
- all code should be under namespace llvmpg, except code for unit testing
- prefer using namespace alias over using namespace delcarations or using the full name of a namespace, prefer using a single character to
  represent the alias of a namespace
- declare namespace alias outside of any namespace block
- namespace llvmpg should be aliased as pg
- member functions definitions having only 3 lines of code or less should be placed inside the definition of the class, except for static member functions
- definition of static member variables should always go outside of the class definition
- prefer using the curiously recurring template pattern over using virtual functions
- if a function parameter is used as output of the function, it should be listed before other parameters used as input to the function
- input parameter to a function should always be const value or const reference
- filenames should always start with lower case character


## requirement for each commit
- do not include any binary executable object in pull request
- project should build successfully
- unit test should all pass
- remove files that are executable from the pull request

## deployment flow
- build: `cmake . && make`

## repository structure
- `src/lexer`: source for lexing a programming language
- `src/parser`: source code for parsing a programming langauge
- `src/codegen`: source code for generating llvm code from a high level programming langauge
- `src/app`: contains main functions and executables this repo
- `src/util`: utility directory used by all other directories
- `test`: unit tests

## key guides
- [the LLVM programmer manual](https://llvm.org/docs/ProgrammersManual.html)
- [LLVM front end tutorial](https://llvm.org/docs/tutorial/)

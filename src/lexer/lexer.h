#ifndef LEXER_H
#define LEXER_H

#include <string>

namespace llvmpg {
namespace k {

// The lexer returns tokens [0-255] if it is an unknown character, otherwise one
// of these for known things.
enum Token : int {
  tok_eof = -1,

  // commands
  tok_def = -2,
  tok_extern = -3,

  // primary
  tok_identifier = -4,
  tok_number = -5,
};

// Global variables for lexer state
extern std::string IdentifierStr; // Filled in if tok_identifier
extern double NumVal;             // Filled in if tok_number

/// gettok - Return the next token from standard input.
int gettok();

} // namespace k
} // namespace llvmpg

#endif
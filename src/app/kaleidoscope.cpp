#include <lexer/lexer.h>
#include <parser/parser.h>
#include <cstdio>

namespace pg = llvmpg;
namespace k = pg::k;

//===----------------------------------------------------------------------===//
// Top-Level parsing
//===----------------------------------------------------------------------===//

void HandleDefinition() {
  if (k::ParseDefinition()) {
    fprintf(stderr, "Parsed a function definition.\n");
  } else {
    // Skip token for error recovery.
    k::getNextToken();
  }
}

void HandleExtern() {
  if (k::ParseExtern()) {
    fprintf(stderr, "Parsed an extern\n");
  } else {
    // Skip token for error recovery.
    k::getNextToken();
  }
}

void HandleTopLevelExpression() {
  // Evaluate a top-level expression into an anonymous function.
  if (k::ParseTopLevelExpr()) {
    fprintf(stderr, "Parsed a top-level expr\n");
  } else {
    // Skip token for error recovery.
    k::getNextToken();
  }
}

/// top ::= definition | external | expression | ';'
void MainLoop() {
  while (true) {
    fprintf(stderr, "ready> ");
    switch (k::CurTok) {
    case k::tok_eof:
      return;
    case ';': // ignore top-level semicolons.
      k::getNextToken();
      break;
    case k::tok_def:
      HandleDefinition();
      break;
    case k::tok_extern:
      HandleExtern();
      break;
    default:
      HandleTopLevelExpression();
      break;
    }
  }
}

//===----------------------------------------------------------------------===//
// Main driver code.
//===----------------------------------------------------------------------===//

int main() {
  // Install standard binary operators.
  // 1 is lowest precedence.
  k::BinopPrecedence['<'] = 10;
  k::BinopPrecedence['+'] = 20;
  k::BinopPrecedence['-'] = 20;
  k::BinopPrecedence['*'] = 40; // highest.

  // Prime the first token.
  fprintf(stderr, "ready> ");
  k::getNextToken();

  // Run the main "interpreter loop" now.
  MainLoop();

  return 0;
}

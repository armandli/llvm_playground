#include <lexer/lexer.h>
#include <parser/parser.h>
#include <codegen/codegen.h>
#include <llvm/Support/raw_ostream.h>
#include <cstdio>

namespace s = std;
namespace pg = llvmpg;
namespace k = pg::k;

// Forward declarations
s::unique_ptr<k::ExprAST> ParseExpressionCodegen();
s::unique_ptr<k::ExprAST> ParseParenExprCodegen();

//===----------------------------------------------------------------------===//
// Codegen-specific parsing overrides
//===----------------------------------------------------------------------===//

// Override the parser functions to return codegen versions
s::unique_ptr<k::ExprAST> ParseNumberExprCodegen() {
  auto Result = k::CreateNumberExprCodegen(k::NumVal);
  k::getNextToken(); // consume the number
  return s::move(Result);
}

s::unique_ptr<k::ExprAST> ParseIdentifierExprCodegen() {
  s::string IdName = k::IdentifierStr;

  k::getNextToken(); // eat identifier.

  if (k::CurTok != '(') // Simple variable ref.
    return k::CreateVariableExprCodegen(IdName);

  // Call.
  k::getNextToken(); // eat (
  s::vector<s::unique_ptr<k::ExprAST>> Args;
  if (k::CurTok != ')') {
    while (true) {
      if (auto Arg = ParseExpressionCodegen())
        Args.push_back(s::move(Arg));
      else
        return nullptr;

      if (k::CurTok == ')')
        break;

      if (k::CurTok != ',')
        return k::LogError("Expected ')' or ',' in argument list");
      k::getNextToken();
    }
  }

  // Eat the ')'.
  k::getNextToken();

  return k::CreateCallExprCodegen(IdName, s::move(Args));
}

s::unique_ptr<k::ExprAST> ParsePrimaryCodegen() {
  switch (k::CurTok) {
  default:
    return k::LogError("unknown token when expecting an expression");
  case k::tok_identifier:
    return ParseIdentifierExprCodegen();
  case k::tok_number:
    return ParseNumberExprCodegen();
  case '(':
    return ParseParenExprCodegen();
  }
}

s::unique_ptr<k::ExprAST> ParseParenExprCodegen() {
  k::getNextToken(); // eat (.
  auto V = ParseExpressionCodegen();
  if (not V)
    return nullptr;

  if (k::CurTok != ')')
    return k::LogError("expected ')'");
  k::getNextToken(); // eat ).
  return V;
}

s::unique_ptr<k::ExprAST> ParseBinOpRHSCodegen(int ExprPrec,
                                               s::unique_ptr<k::ExprAST> LHS) {
  // If this is a binop, find its precedence.
  while (true) {
    int TokPrec = k::GetTokPrecedence();

    // If this is a binop that binds at least as tightly as the current binop,
    // consume it, otherwise we are done.
    if (TokPrec < ExprPrec)
      return LHS;

    // Okay, we know this is a binop.
    int BinOp = k::CurTok;
    k::getNextToken(); // eat binop

    // Parse the primary expression after the binary operator.
    auto RHS = ParsePrimaryCodegen();
    if (not RHS)
      return nullptr;

    // If BinOp binds less tightly with RHS than the operator after RHS, let
    // the pending operator take RHS as its LHS.
    int NextPrec = k::GetTokPrecedence();
    if (TokPrec < NextPrec) {
      RHS = ParseBinOpRHSCodegen(TokPrec + 1, s::move(RHS));
      if (not RHS)
        return nullptr;
    }

    // Merge LHS/RHS.
    LHS = k::CreateBinaryExprCodegen(BinOp, s::move(LHS), s::move(RHS));
  }
}

s::unique_ptr<k::ExprAST> ParseExpressionCodegen() {
  auto LHS = ParsePrimaryCodegen();
  if (not LHS)
    return nullptr;

  return ParseBinOpRHSCodegen(0, s::move(LHS));
}

s::unique_ptr<k::PrototypeAST> ParsePrototypeCodegen() {
  if (k::CurTok != k::tok_identifier)
    return k::LogErrorP("Expected function name in prototype");

  s::string FnName = k::IdentifierStr;
  k::getNextToken();

  if (k::CurTok != '(')
    return k::LogErrorP("Expected '(' in prototype");

  s::vector<s::string> ArgNames;
  while (k::getNextToken() == k::tok_identifier)
    ArgNames.push_back(k::IdentifierStr);
  if (k::CurTok != ')')
    return k::LogErrorP("Expected ')' in prototype");

  // success.
  k::getNextToken(); // eat ')'.

  return k::CreatePrototypeCodegen(FnName, s::move(ArgNames));
}

s::unique_ptr<k::FunctionAST> ParseDefinitionCodegen() {
  k::getNextToken(); // eat def.
  auto Proto = ParsePrototypeCodegen();
  if (not Proto)
    return nullptr;

  if (auto E = ParseExpressionCodegen())
    return k::CreateFunctionCodegen(s::move(Proto), s::move(E));
  return nullptr;
}

s::unique_ptr<k::FunctionAST> ParseTopLevelExprCodegen() {
  if (auto E = ParseExpressionCodegen()) {
    // Make an anonymous proto.
    auto Proto = k::CreatePrototypeCodegen("__anon_expr",
                                           s::vector<s::string>());
    return k::CreateFunctionCodegen(s::move(Proto), s::move(E));
  }
  return nullptr;
}

s::unique_ptr<k::PrototypeAST> ParseExternCodegen() {
  k::getNextToken(); // eat extern.
  return ParsePrototypeCodegen();
}

//===----------------------------------------------------------------------===//
// Top-Level parsing and JIT Driver
//===----------------------------------------------------------------------===//

void HandleDefinition() {
  if (auto FnAST = ParseDefinitionCodegen()) {
    if (auto *FnIR = FnAST->codegen()) {
      fprintf(stderr, "Read function definition:");
      FnIR->print(llvm::errs());
      fprintf(stderr, "\n");
    }
  } else {
    // Skip token for error recovery.
    k::getNextToken();
  }
}

void HandleExtern() {
  if (auto ProtoAST = ParseExternCodegen()) {
    if (auto *FnIR = ProtoAST->codegen()) {
      fprintf(stderr, "Read extern: ");
      FnIR->print(llvm::errs());
      fprintf(stderr, "\n");
    }
  } else {
    // Skip token for error recovery.
    k::getNextToken();
  }
}

void HandleTopLevelExpression() {
  // Evaluate a top-level expression into an anonymous function.
  if (auto FnAST = ParseTopLevelExprCodegen()) {
    if (auto *FnIR = FnAST->codegen()) {
      fprintf(stderr, "Read top-level expression:");
      FnIR->print(llvm::errs());
      fprintf(stderr, "\n");

      // Remove the anonymous expression.
      FnIR->eraseFromParent();
    }
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

  // Make the module, which holds all the code.
  k::InitializeModule();

  // Run the main "interpreter loop" now.
  MainLoop();

  // Print out all of the generated code.
  k::TheModule->print(llvm::errs(), nullptr);

  return 0;
}
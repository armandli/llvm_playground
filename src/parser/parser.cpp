#include <parser/parser.h>
#include <cctype>
#include <cstdio>

namespace llvmpg {
namespace k {

namespace s = std;

//===----------------------------------------------------------------------===//
// Parser
//===----------------------------------------------------------------------===//

/// CurTok/getNextToken - Provide a simple token buffer.  CurTok is the current
/// token the parser is looking at.  getNextToken reads another token from the
/// lexer and updates CurTok with its results.
int CurTok;
int getNextToken() { return CurTok = gettok(); }

/// BinopPrecedence - This holds the precedence for each binary operator that is
/// defined.
s::map<char, int> BinopPrecedence;

/// GetTokPrecedence - Get the precedence of the pending binary operator token.
int GetTokPrecedence() {
  if (not isascii(CurTok))
    return -1;

  // Make sure it's a declared binop.
  int TokPrec = BinopPrecedence[CurTok];
  if (TokPrec <= 0)
    return -1;
  return TokPrec;
}

/// LogError* - These are little helper functions for error handling.
s::unique_ptr<ExprAST> LogError(const char *Str) {
  fprintf(stderr, "Error: %s\n", Str);
  return nullptr;
}
s::unique_ptr<PrototypeAST> LogErrorP(const char *Str) {
  LogError(Str);
  return nullptr;
}

/// numberexpr ::= number
s::unique_ptr<ExprAST> ParseNumberExpr() {
  auto Result = s::make_unique<NumberExprAST>(NumVal);
  getNextToken(); // consume the number
  return s::move(Result);
}

/// parenexpr ::= '(' expression ')'
s::unique_ptr<ExprAST> ParseParenExpr() {
  getNextToken(); // eat (.
  auto V = ParseExpression();
  if (not V)
    return nullptr;

  if (CurTok != ')')
    return LogError("expected ')'");
  getNextToken(); // eat ).
  return V;
}

/// identifierexpr
///   ::= identifier
///   ::= identifier '(' expression* ')'
s::unique_ptr<ExprAST> ParseIdentifierExpr() {
  s::string IdName = IdentifierStr;

  getNextToken(); // eat identifier.

  if (CurTok != '(') // Simple variable ref.
    return s::make_unique<VariableExprAST>(IdName);

  // Call.
  getNextToken(); // eat (
  s::vector<s::unique_ptr<ExprAST>> Args;
  if (CurTok != ')') {
    while (true) {
      if (auto Arg = ParseExpression())
        Args.push_back(s::move(Arg));
      else
        return nullptr;

      if (CurTok == ')')
        break;

      if (CurTok != ',')
        return LogError("Expected ')' or ',' in argument list");
      getNextToken();
    }
  }

  // Eat the ')'.
  getNextToken();

  return s::make_unique<CallExprAST>(IdName, s::move(Args));
}

/// primary
///   ::= identifierexpr
///   ::= numberexpr
///   ::= parenexpr
s::unique_ptr<ExprAST> ParsePrimary() {
  switch (CurTok) {
  default:
    return LogError("unknown token when expecting an expression");
  case tok_identifier:
    return ParseIdentifierExpr();
  case tok_number:
    return ParseNumberExpr();
  case '(':
    return ParseParenExpr();
  }
}

/// binoprhs
///   ::= ('+' primary)*
s::unique_ptr<ExprAST> ParseBinOpRHS(int ExprPrec,
                                     s::unique_ptr<ExprAST> LHS) {
  // If this is a binop, find its precedence.
  while (true) {
    int TokPrec = GetTokPrecedence();

    // If this is a binop that binds at least as tightly as the current binop,
    // consume it, otherwise we are done.
    if (TokPrec < ExprPrec)
      return LHS;

    // Okay, we know this is a binop.
    int BinOp = CurTok;
    getNextToken(); // eat binop

    // Parse the primary expression after the binary operator.
    auto RHS = ParsePrimary();
    if (not RHS)
      return nullptr;

    // If BinOp binds less tightly with RHS than the operator after RHS, let
    // the pending operator take RHS as its LHS.
    int NextPrec = GetTokPrecedence();
    if (TokPrec < NextPrec) {
      RHS = ParseBinOpRHS(TokPrec + 1, s::move(RHS));
      if (not RHS)
        return nullptr;
    }

    // Merge LHS/RHS.
    LHS = s::make_unique<BinaryExprAST>(BinOp, s::move(LHS), s::move(RHS));
  }
}

/// expression
///   ::= primary binoprhs
///
s::unique_ptr<ExprAST> ParseExpression() {
  auto LHS = ParsePrimary();
  if (not LHS)
    return nullptr;

  return ParseBinOpRHS(0, s::move(LHS));
}

/// prototype
///   ::= id '(' id* ')'
s::unique_ptr<PrototypeAST> ParsePrototype() {
  if (CurTok != tok_identifier)
    return LogErrorP("Expected function name in prototype");

  s::string FnName = IdentifierStr;
  getNextToken();

  if (CurTok != '(')
    return LogErrorP("Expected '(' in prototype");

  s::vector<s::string> ArgNames;
  while (getNextToken() == tok_identifier)
    ArgNames.push_back(IdentifierStr);
  if (CurTok != ')')
    return LogErrorP("Expected ')' in prototype");

  // success.
  getNextToken(); // eat ')'.

  return s::make_unique<PrototypeAST>(FnName, s::move(ArgNames));
}

/// definition ::= 'def' prototype expression
s::unique_ptr<FunctionAST> ParseDefinition() {
  getNextToken(); // eat def.
  auto Proto = ParsePrototype();
  if (not Proto)
    return nullptr;

  if (auto E = ParseExpression())
    return s::make_unique<FunctionAST>(s::move(Proto), s::move(E));
  return nullptr;
}

/// toplevelexpr ::= expression
s::unique_ptr<FunctionAST> ParseTopLevelExpr() {
  if (auto E = ParseExpression()) {
    // Make an anonymous proto.
    auto Proto = s::make_unique<PrototypeAST>("__anon_expr",
                                              s::vector<s::string>());
    return s::make_unique<FunctionAST>(s::move(Proto), s::move(E));
  }
  return nullptr;
}

/// external ::= 'extern' prototype
s::unique_ptr<PrototypeAST> ParseExtern() {
  getNextToken(); // eat extern.
  return ParsePrototype();
}

} // namespace k
} // namespace llvmpg
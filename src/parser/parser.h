#ifndef PARSER_H
#define PARSER_H

#include <lexer/lexer.h>
#include <memory>
#include <string>
#include <vector>
#include <map>

namespace llvm {
  class Value;
  class Function;
}

namespace llvmpg {
namespace k {

namespace s = std;

//===----------------------------------------------------------------------===//
// Abstract Syntax Tree (aka Parse Tree)
//===----------------------------------------------------------------------===//

/// ExprAST - Base class for all expression nodes.
struct ExprAST {
  virtual ~ExprAST() = default;
  virtual llvm::Value *codegen() { return nullptr; }
};

/// NumberExprAST - Expression class for numeric literals like "1.0".
struct NumberExprAST : public ExprAST {
  double Val;

  NumberExprAST(double Val) : Val(Val) {}
  llvm::Value *codegen() override { return nullptr; }
};

/// VariableExprAST - Expression class for referencing a variable, like "a".
struct VariableExprAST : public ExprAST {
  s::string Name;

  VariableExprAST(const s::string &Name) : Name(Name) {}
  llvm::Value *codegen() override { return nullptr; }
};

/// BinaryExprAST - Expression class for a binary operator.
struct BinaryExprAST : public ExprAST {
  char Op;
  s::unique_ptr<ExprAST> LHS, RHS;

  BinaryExprAST(char Op, s::unique_ptr<ExprAST> LHS,
                s::unique_ptr<ExprAST> RHS)
      : Op(Op), LHS(s::move(LHS)), RHS(s::move(RHS)) {}
  llvm::Value *codegen() override { return nullptr; }
};

/// CallExprAST - Expression class for function calls.
struct CallExprAST : public ExprAST {
  s::string Callee;
  s::vector<s::unique_ptr<ExprAST>> Args;

  CallExprAST(const s::string &Callee,
              s::vector<s::unique_ptr<ExprAST>> Args)
      : Callee(Callee), Args(s::move(Args)) {}
  llvm::Value *codegen() override { return nullptr; }
};

/// PrototypeAST - This class represents the "prototype" for a function,
/// which captures its name, and its argument names (thus implicitly the number
/// of arguments the function takes).
struct PrototypeAST {
  s::string Name;
  s::vector<s::string> Args;

  PrototypeAST(const s::string &Name, s::vector<s::string> Args)
      : Name(Name), Args(s::move(Args)) {}

  virtual llvm::Function *codegen() { return nullptr; }
  const s::string &getName() const { return Name; }
};

/// FunctionAST - This class represents a function definition itself.
struct FunctionAST {
  s::unique_ptr<PrototypeAST> Proto;
  s::unique_ptr<ExprAST> Body;

  FunctionAST(s::unique_ptr<PrototypeAST> Proto,
              s::unique_ptr<ExprAST> Body)
      : Proto(s::move(Proto)), Body(s::move(Body)) {}
  virtual llvm::Function *codegen() { return nullptr; }
};

//===----------------------------------------------------------------------===//
// Parser
//===----------------------------------------------------------------------===//

/// CurTok/getNextToken - Provide a simple token buffer.  CurTok is the current
/// token the parser is looking at.  getNextToken reads another token from the
/// lexer and updates CurTok with its results.
extern int CurTok;
int getNextToken();

/// BinopPrecedence - This holds the precedence for each binary operator that is
/// defined.
extern s::map<char, int> BinopPrecedence;

/// GetTokPrecedence - Get the precedence of the pending binary operator token.
int GetTokPrecedence();

/// LogError* - These are little helper functions for error handling.
s::unique_ptr<ExprAST> LogError(const char *Str);
s::unique_ptr<PrototypeAST> LogErrorP(const char *Str);

s::unique_ptr<ExprAST> ParseExpression();
s::unique_ptr<ExprAST> ParseNumberExpr();
s::unique_ptr<ExprAST> ParseParenExpr();
s::unique_ptr<ExprAST> ParseIdentifierExpr();
s::unique_ptr<ExprAST> ParsePrimary();
s::unique_ptr<ExprAST> ParseBinOpRHS(int ExprPrec, s::unique_ptr<ExprAST> LHS);
s::unique_ptr<PrototypeAST> ParsePrototype();
s::unique_ptr<FunctionAST> ParseDefinition();
s::unique_ptr<FunctionAST> ParseTopLevelExpr();
s::unique_ptr<PrototypeAST> ParseExtern();

} // namespace k
} // namespace llvmpg

#endif
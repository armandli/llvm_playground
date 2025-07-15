#ifndef CODEGEN_H
#define CODEGEN_H

#include <parser/parser.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Value.h>
#include <llvm/IR/Function.h>
#include <memory>
#include <map>
#include <string>

namespace llvmpg {
namespace k {

namespace s = std;

//===----------------------------------------------------------------------===//
// Code Generation
//===----------------------------------------------------------------------===//

extern s::unique_ptr<llvm::LLVMContext> TheContext;
extern s::unique_ptr<llvm::Module> TheModule;
extern s::unique_ptr<llvm::IRBuilder<>> Builder;
extern s::map<s::string, llvm::Value *> NamedValues;

llvm::Value *LogErrorV(const char *Str);
void InitializeModule();

//===----------------------------------------------------------------------===//
// Codegen AST Extensions
//===----------------------------------------------------------------------===//

/// NumberExprAST - Expression class for numeric literals like "1.0".
struct NumberExprCodegen : public NumberExprAST {
  NumberExprCodegen(double Val) : NumberExprAST(Val) {}
  llvm::Value *codegen() override;
};

/// VariableExprAST - Expression class for referencing a variable, like "a".
struct VariableExprCodegen : public VariableExprAST {
  VariableExprCodegen(const s::string &Name) : VariableExprAST(Name) {}
  llvm::Value *codegen() override;
};

/// BinaryExprAST - Expression class for a binary operator.
struct BinaryExprCodegen : public BinaryExprAST {
  BinaryExprCodegen(char Op, s::unique_ptr<ExprAST> LHS,
                    s::unique_ptr<ExprAST> RHS)
      : BinaryExprAST(Op, s::move(LHS), s::move(RHS)) {}
  llvm::Value *codegen() override;
};

/// CallExprAST - Expression class for function calls.
struct CallExprCodegen : public CallExprAST {
  CallExprCodegen(const s::string &Callee,
                  s::vector<s::unique_ptr<ExprAST>> Args)
      : CallExprAST(Callee, s::move(Args)) {}
  llvm::Value *codegen() override;
};

/// PrototypeAST - This class represents the "prototype" for a function,
/// which captures its name, and its argument names (thus implicitly the number
/// of arguments the function takes).
struct PrototypeCodegen : public PrototypeAST {
  PrototypeCodegen(const s::string &Name, s::vector<s::string> Args)
      : PrototypeAST(Name, s::move(Args)) {}
  llvm::Function *codegen() override;
};

/// FunctionAST - This class represents a function definition itself.
struct FunctionCodegen : public FunctionAST {
  FunctionCodegen(s::unique_ptr<PrototypeAST> Proto,
                  s::unique_ptr<ExprAST> Body)
      : FunctionAST(s::move(Proto), s::move(Body)) {}
  llvm::Function *codegen() override;
};

// Helper functions to create codegen versions of AST nodes
s::unique_ptr<ExprAST> CreateNumberExprCodegen(double Val);
s::unique_ptr<ExprAST> CreateVariableExprCodegen(const s::string &Name);
s::unique_ptr<ExprAST> CreateBinaryExprCodegen(char Op, s::unique_ptr<ExprAST> LHS,
                                               s::unique_ptr<ExprAST> RHS);
s::unique_ptr<ExprAST> CreateCallExprCodegen(const s::string &Callee,
                                             s::vector<s::unique_ptr<ExprAST>> Args);
s::unique_ptr<PrototypeAST> CreatePrototypeCodegen(const s::string &Name,
                                                   s::vector<s::string> Args);
s::unique_ptr<FunctionAST> CreateFunctionCodegen(s::unique_ptr<PrototypeAST> Proto,
                                                 s::unique_ptr<ExprAST> Body);

} // namespace k
} // namespace llvmpg

#endif
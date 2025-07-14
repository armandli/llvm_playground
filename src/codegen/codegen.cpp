#include <codegen/codegen.h>
#include <llvm/ADT/APFloat.h>
#include <llvm/ADT/STLExtras.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Verifier.h>
#include <cstdio>

namespace llvmpg {
namespace k {

namespace s = std;

//===----------------------------------------------------------------------===//
// Code Generation
//===----------------------------------------------------------------------===//

s::unique_ptr<llvm::LLVMContext> TheContext;
s::unique_ptr<llvm::Module> TheModule;
s::unique_ptr<llvm::IRBuilder<>> Builder;
s::map<s::string, llvm::Value *> NamedValues;

llvm::Value *LogErrorV(const char *Str) {
  LogError(Str);
  return nullptr;
}

void InitializeModule() {
  // Open a new context and module.
  TheContext = s::make_unique<llvm::LLVMContext>();
  TheModule = s::make_unique<llvm::Module>("my cool jit", *TheContext);

  // Create a new builder for the module.
  Builder = s::make_unique<llvm::IRBuilder<>>(*TheContext);
}

llvm::Value *NumberExprCodegen::codegen() {
  return llvm::ConstantFP::get(*TheContext, llvm::APFloat(Val));
}

llvm::Value *VariableExprCodegen::codegen() {
  // Look this variable up in the function.
  llvm::Value *V = NamedValues[Name];
  if (not V)
    return LogErrorV("Unknown variable name");
  return V;
}

llvm::Value *BinaryExprCodegen::codegen() {
  llvm::Value *L = LHS->codegen();
  llvm::Value *R = RHS->codegen();
  if (not L or not R)
    return nullptr;

  switch (Op) {
  case '+':
    return Builder->CreateFAdd(L, R, "addtmp");
  case '-':
    return Builder->CreateFSub(L, R, "subtmp");
  case '*':
    return Builder->CreateFMul(L, R, "multmp");
  case '<':
    L = Builder->CreateFCmpULT(L, R, "cmptmp");
    // Convert bool 0/1 to double 0.0 or 1.0
    return Builder->CreateUIToFP(L, llvm::Type::getDoubleTy(*TheContext), "booltmp");
  default:
    return LogErrorV("invalid binary operator");
  }
}

llvm::Value *CallExprCodegen::codegen() {
  // Look up the name in the global module table.
  llvm::Function *CalleeF = TheModule->getFunction(Callee);
  if (not CalleeF)
    return LogErrorV("Unknown function referenced");

  // If argument mismatch error.
  if (CalleeF->arg_size() != Args.size())
    return LogErrorV("Incorrect # arguments passed");

  s::vector<llvm::Value *> ArgsV;
  for (unsigned i = 0, e = Args.size(); i != e; ++i) {
    ArgsV.push_back(Args[i]->codegen());
    if (not ArgsV.back())
      return nullptr;
  }

  return Builder->CreateCall(CalleeF, ArgsV, "calltmp");
}

llvm::Function *PrototypeCodegen::codegen() {
  // Make the function type:  double(double,double) etc.
  s::vector<llvm::Type *> Doubles(Args.size(), llvm::Type::getDoubleTy(*TheContext));
  llvm::FunctionType *FT =
      llvm::FunctionType::get(llvm::Type::getDoubleTy(*TheContext), Doubles, false);

  llvm::Function *F =
      llvm::Function::Create(FT, llvm::Function::ExternalLinkage, Name, TheModule.get());

  // Set names for all arguments.
  unsigned Idx = 0;
  for (auto &Arg : F->args())
    Arg.setName(Args[Idx++]);

  return F;
}

llvm::Function *FunctionCodegen::codegen() {
  // First, check for an existing function from a previous 'extern' declaration.
  llvm::Function *TheFunction = TheModule->getFunction(Proto->getName());

  if (not TheFunction)
    TheFunction = Proto->codegen();

  if (not TheFunction)
    return nullptr;

  // Create a new basic block to start insertion into.
  llvm::BasicBlock *BB = llvm::BasicBlock::Create(*TheContext, "entry", TheFunction);
  Builder->SetInsertPoint(BB);

  // Record the function arguments in the NamedValues map.
  NamedValues.clear();
  for (auto &Arg : TheFunction->args())
    NamedValues[s::string(Arg.getName())] = &Arg;

  if (llvm::Value *RetVal = Body->codegen()) {
    // Finish off the function.
    Builder->CreateRet(RetVal);

    // Validate the generated code, checking for consistency.
    verifyFunction(*TheFunction);

    return TheFunction;
  }

  // Error reading body, remove function.
  TheFunction->eraseFromParent();
  return nullptr;
}

// Helper functions to create codegen versions of AST nodes
s::unique_ptr<ExprAST> CreateNumberExprCodegen(double Val) {
  return s::make_unique<NumberExprCodegen>(Val);
}

s::unique_ptr<ExprAST> CreateVariableExprCodegen(const s::string &Name) {
  return s::make_unique<VariableExprCodegen>(Name);
}

s::unique_ptr<ExprAST> CreateBinaryExprCodegen(char Op, s::unique_ptr<ExprAST> LHS,
                                               s::unique_ptr<ExprAST> RHS) {
  return s::make_unique<BinaryExprCodegen>(Op, s::move(LHS), s::move(RHS));
}

s::unique_ptr<ExprAST> CreateCallExprCodegen(const s::string &Callee,
                                             s::vector<s::unique_ptr<ExprAST>> Args) {
  return s::make_unique<CallExprCodegen>(Callee, s::move(Args));
}

s::unique_ptr<PrototypeAST> CreatePrototypeCodegen(const s::string &Name,
                                                   s::vector<s::string> Args) {
  return s::make_unique<PrototypeCodegen>(Name, s::move(Args));
}

s::unique_ptr<FunctionAST> CreateFunctionCodegen(s::unique_ptr<PrototypeAST> Proto,
                                                 s::unique_ptr<ExprAST> Body) {
  return s::make_unique<FunctionCodegen>(s::move(Proto), s::move(Body));
}

} // namespace k
} // namespace llvmpg
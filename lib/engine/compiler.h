
#ifndef TSUGU_ENGINE_COMPILER_H
#define TSUGU_ENGINE_COMPILER_H

#include <tsugu/core/ast.h>
#include <tsugu/core/tyenv.h>
#include <llvm/IR/IRBuilder.h>

namespace tsugu {

class Compiler {
 public:
  Compiler() : context(), builder(context), module(nullptr) {}
  virtual ~Compiler() {}

  int32_t run(tsg_ast_t* ast, tsg_tyenv_t* env);

 private:
  llvm::LLVMContext context;
  llvm::IRBuilder<> builder;
  llvm::Module* module;
  tsg_tyenv_t* tyenv;

  std::vector<std::vector<llvm::Value*>> value_table;
  std::vector<llvm::Function*> function_table;

  void enterScope(size_t size);
  void leaveScope(void);
  void insert(tsg_decl_t* decl, llvm::Value* value);
  llvm::Value* lookup(tsg_decl_t* decl);

  llvm::Type* convTy(tsg_type_t* type);
  llvm::FunctionType* convFuncTy(tsg_type_t* type);
  std::vector<llvm::Type*> convTyArr(tsg_type_arr_t* arr);

  void buildAst(tsg_ast_t* ast);
  llvm::Function* fetchFunc(tsg_func_t* func);
  llvm::Function* buildFunc(tsg_func_t* func);
  llvm::Value* buildBlock(tsg_block_t* block);

  llvm::Value* buildStmt(tsg_stmt_t* stmt);
  llvm::Value* buildStmtVal(tsg_stmt_t* stmt);
  llvm::Value* buildStmtExpr(tsg_stmt_t* stmt);

  llvm::Value* buildExpr(tsg_expr_t* expr);
  llvm::Value* buildExprIfelse(tsg_expr_t* expr);
  llvm::Value* buildExprBinary(tsg_expr_t* expr);
  llvm::Value* buildExprCall(tsg_expr_t* expr);
  llvm::Value* buildExprVariable(tsg_expr_t* expr);
  llvm::Value* buildExprNumber(tsg_expr_t* expr);
};

}  // namespace tsugu

#endif

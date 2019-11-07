/*----------------------------------- vi: set ft=cpp ts=2 sw=2 et: --*-cpp-*--*/
/**
 * @file compiler.h
 *
 ** --------------------------------------------------------------------------*/

#ifndef TSUGU_ENGINE_COMPILER_H
#define TSUGU_ENGINE_COMPILER_H

#include "function_table.h"
#include <tsugu/core/ast.h>
#include <tsugu/core/tyenv.h>
#include <llvm/IR/IRBuilder.h>

namespace tsugu {

class Compiler {
 public:
  Compiler();
  virtual ~Compiler();

  int32_t run(tsg_ast_t* ast);

 private:
  llvm::LLVMContext context;
  llvm::IRBuilder<> builder;
  llvm::Module* module;

  tsg_tyenv_t* tyenv;
  tsg_frame_t* frametype;
  llvm::Value* frameptr;
  FunctionTable* function_table;

  void store(tsg_member_t* member, llvm::Value* value);
  llvm::Value* load(tsg_member_t* member);
  llvm::Value* createObjPtr(tsg_member_t* member);
  llvm::Value* createObjPtrRaw(int32_t depth, int32_t index);

  llvm::Type* convTy(tsg_type_t* type);
  llvm::FunctionType* convFuncTy(tsg_type_t* type);
  llvm::StructType* convFrameTy(tsg_frame_t* frame);
  void convTyArr(std::vector<llvm::Type*>& types, tsg_type_arr_t* arr);

  void buildAst(tsg_ast_t* ast, tsg_tyenv_t* env);
  llvm::Function* fetchFunc(tsg_func_t* func, tsg_tyenv_t* env);
  llvm::Function* buildFunc(tsg_func_t* func, tsg_tyenv_t* env);
  llvm::Value* buildBlock(tsg_block_t* block);
  void buildFuncList(tsg_func_list_t* funcs);
  llvm::Value* buildStmtList(tsg_stmt_list_t* stmts);

  llvm::Value* buildStmt(tsg_stmt_t* stmt);
  llvm::Value* buildStmtVal(tsg_stmt_t* stmt);
  llvm::Value* buildStmtExpr(tsg_stmt_t* stmt);

  llvm::Value* buildExpr(tsg_expr_t* expr);
  llvm::Value* buildExprBinary(tsg_expr_t* expr);
  llvm::Value* buildExprCall(tsg_expr_t* expr);
  llvm::Value* buildExprIfelse(tsg_expr_t* expr);
  llvm::Value* buildExprIdent(tsg_expr_t* expr);
  llvm::Value* buildExprNumber(tsg_expr_t* expr);
};

}  // namespace tsugu

#endif

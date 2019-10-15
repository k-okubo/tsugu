/*----------------------------------- vi: set ft=cpp ts=2 sw=2 et: --*-cpp-*--*/
/**
 * @file compiler.cpp
 *
 ** --------------------------------------------------------------------------*/

#include "compiler.h"

#include <tsugu/core/platform.h>
#include <tsugu/core/tymap.h>
#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Support/TargetSelect.h>

using namespace tsugu;

typedef int32_t (*main_func_t)(void);

int32_t Compiler::run(tsg_ast_t* ast, tsg_tyenv_t* env) {
  this->root_env = env;
  this->func_env = nullptr;

  llvm::InitializeNativeTarget();
  llvm::InitializeNativeTargetAsmPrinter();

  auto moduleOwner = llvm::make_unique<llvm::Module>("main_module", context);
  module = moduleOwner.get();

  buildAst(ast);

  if (llvm::verifyModule(*module, &(llvm::errs()))) {
    llvm::errs() << "verifyModule Failed\n";
    return -1;
  }

  module->print(llvm::errs(), nullptr);

  std::string err;
  auto engine =
      llvm::EngineBuilder(std::move(moduleOwner)).setErrorStr(&err).create();

  if (!engine) {
    llvm::errs() << err << "\n";
    return -1;
  }

  auto f = (main_func_t)engine->getFunctionAddress("main");
  if (!f) {
    llvm::errs() << "function not found\n";
    return -1;
  }
  int32_t result = f();

  engine->finalizeObject();
  delete engine;

  return result;
}

void Compiler::enterScope(size_t size) {
  value_table.push_back(std::vector<llvm::Value*>(size, nullptr));
}

void Compiler::leaveScope(void) {
  value_table.pop_back();
}

void Compiler::insert(tsg_decl_t* decl, llvm::Value* value) {
  int32_t depth = decl->depth;
  int32_t index = decl->index;

  value_table[depth][index] = value;
}

llvm::Value* Compiler::lookup(tsg_decl_t* decl) {
  int32_t depth = decl->depth;
  int32_t index = decl->index;

  return value_table[depth][index];
}

llvm::Type* Compiler::convTy(tsg_type_t* type) {
  switch (type->kind) {
    case TSG_TYPE_BOOL:
      return builder.getInt1Ty();

    case TSG_TYPE_INT:
      return builder.getInt32Ty();

    case TSG_TYPE_FUNC:
      return convFuncTy(type)->getPointerTo();

    case TSG_TYPE_POLY:
      return builder.getInt32Ty();

    case TSG_TYPE_PEND:
      return nullptr;
  }

  return nullptr;
}

llvm::FunctionType* Compiler::convFuncTy(tsg_type_t* type) {
  auto ret_type = convTy(type->func.ret);
  auto param_types = convTyArr(type->func.params);
  auto func_type = llvm::FunctionType::get(ret_type, param_types, false);

  return func_type;
}

std::vector<llvm::Type*> Compiler::convTyArr(tsg_type_arr_t* arr) {
  std::vector<llvm::Type*> result;

  tsg_type_t** p = arr->elem;
  tsg_type_t** end = arr->elem + arr->size;

  while (p < end) {
    result.push_back(convTy(*p));
    p++;
  }

  return result;
}

void Compiler::buildAst(tsg_ast_t* ast) {
  enterScope(ast->functions->size);
  function_table = new FunctionTable();
  tsg_type_t* main_type = nullptr;

  auto node = ast->functions->head;
  while (node) {
    insert(node->func->decl, builder.getInt32(0));
    if (tsg_memcmp(node->func->decl->name->buffer, "main", 4) == 0) {
      main_type = tsg_tyenv_get(root_env, node->func->decl->type_id);
    }
    node = node->next;
  }

  tsg_type_arr_t* main_args = tsg_type_arr_create(0);
  tsg_tyenv_t* main_env = tsg_tymap_get(main_type->poly.tymap, main_args);

  fetchFunc(main_type->poly.func, main_env);
  leaveScope();
}

llvm::Function* Compiler::fetchFunc(tsg_func_t* func, tsg_tyenv_t* env) {
  llvm::Function* llvm_func = function_table->get(func, env);
  if (llvm_func) {
    return llvm_func;
  }

  llvm_func = buildFunc(func, env);
  return llvm_func;
}

llvm::Function* Compiler::buildFunc(tsg_func_t* func, tsg_tyenv_t* env) {
  tsg_decl_t* decl = func->decl;
  auto func_type = tsg_tyenv_get(env, 0);
  auto llvm_func = llvm::Function::Create(convFuncTy(func_type),
                                          llvm::Function::ExternalLinkage,
                                          tsg_ident_cstr(decl->name), module);

  function_table->set(func, env, llvm_func);
  auto body = llvm::BasicBlock::Create(context, "entry", llvm_func);
  builder.SetInsertPoint(body);

  auto prev_scope = std::move(value_table);
  value_table.assign(1, prev_scope[0]);

  auto prev_env = this->func_env;
  this->func_env = env;

  enterScope(func->body->n_decls);

  auto node = func->args->head;
  for (auto& arg : llvm_func->args()) {
    arg.setName(tsg_ident_cstr(node->decl->name));
    insert(node->decl, &arg);
    node = node->next;
  }

  llvm::Value* last_value = buildBlock(func->body);

  leaveScope();

  if (last_value) {
    builder.CreateRet(last_value);
  } else {
    builder.CreateRetVoid();
  }

  if (llvm::verifyFunction(*llvm_func, &(llvm::errs()))) {
    llvm::errs() << "verifyFunction Failed\n";
  }

  this->func_env = prev_env;
  value_table = std::move(prev_scope);

  return llvm_func;
}

llvm::Value* Compiler::buildBlock(tsg_block_t* block) {
  llvm::Value* last_value = nullptr;

  auto node = block->stmts->head;
  while (node) {
    last_value = buildStmt(node->stmt);
    node = node->next;
  }

  return last_value;
}

llvm::Value* Compiler::buildStmt(tsg_stmt_t* stmt) {
  switch (stmt->kind) {
    case TSG_STMT_VAL:
      return buildStmtVal(stmt);
    case TSG_STMT_EXPR:
      return buildStmtExpr(stmt);
  }

  return nullptr;
}

llvm::Value* Compiler::buildStmtVal(tsg_stmt_t* stmt) {
  llvm::Value* value = buildExpr(stmt->val.expr);
  value->setName(tsg_ident_cstr(stmt->val.decl->name));
  insert(stmt->val.decl, value);

  return value;
}

llvm::Value* Compiler::buildStmtExpr(tsg_stmt_t* stmt) {
  return buildExpr(stmt->expr.expr);
}

llvm::Value* Compiler::buildExpr(tsg_expr_t* expr) {
  switch (expr->kind) {
    case TSG_EXPR_IFELSE:
      return buildExprIfelse(expr);
    case TSG_EXPR_BINARY:
      return buildExprBinary(expr);
    case TSG_EXPR_CALL:
      return buildExprCall(expr);
    case TSG_EXPR_VARIABLE:
      return buildExprVariable(expr);
    case TSG_EXPR_NUMBER:
      return buildExprNumber(expr);
  }

  return nullptr;
}

llvm::Value* Compiler::buildExprIfelse(tsg_expr_t* expr) {
  llvm::Function* func = builder.GetInsertBlock()->getParent();
  auto then_block = llvm::BasicBlock::Create(context, "then");
  auto else_block = llvm::BasicBlock::Create(context, "else");
  auto merge_block = llvm::BasicBlock::Create(context, "merge");

  llvm::Value* cond = buildExpr(expr->ifelse.cond);
  builder.CreateCondBr(cond, then_block, else_block);

  enterScope(expr->ifelse.thn->n_decls);
  func->getBasicBlockList().push_back(then_block);
  builder.SetInsertPoint(then_block);
  llvm::Value* then_value = buildBlock(expr->ifelse.thn);
  builder.CreateBr(merge_block);
  then_block = builder.GetInsertBlock();
  leaveScope();

  enterScope(expr->ifelse.els->n_decls);
  func->getBasicBlockList().push_back(else_block);
  builder.SetInsertPoint(else_block);
  llvm::Value* else_value = buildBlock(expr->ifelse.els);
  builder.CreateBr(merge_block);
  else_block = builder.GetInsertBlock();
  leaveScope();

  func->getBasicBlockList().push_back(merge_block);
  builder.SetInsertPoint(merge_block);
  auto phi = builder.CreatePHI(then_value->getType(), 2);
  phi->addIncoming(then_value, then_block);
  phi->addIncoming(else_value, else_block);

  return phi;
}

llvm::Value* Compiler::buildExprBinary(tsg_expr_t* expr) {
  llvm::Value* lhs = buildExpr(expr->binary.lhs);
  llvm::Value* rhs = buildExpr(expr->binary.rhs);

  switch (expr->binary.op) {
    case TSG_TOKEN_EQ:
      return builder.CreateICmpEQ(lhs, rhs);
    case TSG_TOKEN_LT:
      return builder.CreateICmpSLT(lhs, rhs);
    case TSG_TOKEN_GT:
      return builder.CreateICmpSGT(lhs, rhs);
    case TSG_TOKEN_ADD:
      return builder.CreateAdd(lhs, rhs);
    case TSG_TOKEN_SUB:
      return builder.CreateSub(lhs, rhs);
    case TSG_TOKEN_MUL:
      return builder.CreateMul(lhs, rhs);
    case TSG_TOKEN_DIV:
      return builder.CreateSDiv(lhs, rhs);
    default:
      return nullptr;
  }
}

llvm::Value* Compiler::buildExprCall(tsg_expr_t* expr) {
  buildExpr(expr->call.callee);

  std::vector<llvm::Value*> args;
  tsg_type_arr_t* arg_types = tsg_type_arr_create(expr->call.args->size);
  tsg_type_t** p = arg_types->elem;

  auto node = expr->call.args->head;
  while (node) {
    auto value = buildExpr(node->expr);
    args.push_back(value);
    *(p++) = tsg_tyenv_get(func_env, node->expr->type_id);
    node = node->next;
  }

  auto block = builder.GetInsertBlock();
  tsg_type_t* callee_type = tsg_tyenv_get(func_env, expr->call.callee->type_id);
  tsg_tyenv_t* callee_env = tsg_tymap_get(callee_type->poly.tymap, arg_types);
  llvm::Value* callee = fetchFunc(callee_type->poly.func, callee_env);
  builder.SetInsertPoint(block);

  return builder.CreateCall(callee, args);
}

llvm::Value* Compiler::buildExprVariable(tsg_expr_t* expr) {
  return lookup(expr->variable.resolved);
}

llvm::Value* Compiler::buildExprNumber(tsg_expr_t* expr) {
  return builder.getInt32(expr->number.value);
}

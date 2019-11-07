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

Compiler::Compiler()
    : context(),
      builder(context),
      module(nullptr),
      tyenv(nullptr),
      frametype(nullptr),
      frameptr(nullptr),
      function_table(nullptr) {}

Compiler::~Compiler() {}

int32_t Compiler::run(tsg_ast_t* ast) {
  llvm::InitializeNativeTarget();
  llvm::InitializeNativeTargetAsmPrinter();

  auto moduleOwner = llvm::make_unique<llvm::Module>("main_module", context);
  module = moduleOwner.get();

  function_table = new FunctionTable();

  buildAst(ast, ast->tyenv);
  module->print(llvm::errs(), nullptr);

  if (llvm::verifyModule(*module, &(llvm::errs()))) {
    llvm::errs() << "verifyModule Failed\n";
    return -1;
  }

  std::string err;
  auto engine =
      llvm::EngineBuilder(std::move(moduleOwner)).setErrorStr(&err).create();

  if (!engine) {
    llvm::errs() << err << "\n";
    return -1;
  }

  auto f = (main_func_t)engine->getFunctionAddress(
      tsg_ident_cstr(ast->root->decl->name));
  if (!f) {
    llvm::errs() << "function not found\n";
    return -1;
  }
  int32_t result = f();

  engine->finalizeObject();
  delete engine;
  delete function_table;

  return result;
}

void Compiler::store(tsg_member_t* member, llvm::Value* value) {
  builder.CreateStore(value, createObjPtr(member));
}

llvm::Value* Compiler::load(tsg_member_t* member) {
  return builder.CreateLoad(createObjPtr(member));
}

llvm::Value* Compiler::createObjPtr(tsg_member_t* member) {
  return createObjPtrRaw(member->depth, member->index + 1);
}

llvm::Value* Compiler::createObjPtrRaw(int32_t depth, int32_t index) {
  llvm::Value* fp = this->frameptr;
  tsg_frame_t* ft = this->frametype;
  assert(0 <= depth && depth <= ft->depth);

  while (depth < ft->depth) {
    std::vector<llvm::Value*> elem_idx;
    elem_idx.push_back(builder.getInt32(0));
    elem_idx.push_back(builder.getInt32(0));
    fp = builder.CreateLoad(builder.CreateGEP(fp, elem_idx));
    ft = ft->outer;
  }

  // `fp` has a pointer to outer frame as a member, but `ft` does not.
  assert(0 <= index && index < ft->size + 1);

  std::vector<llvm::Value*> elem_idx;
  elem_idx.push_back(builder.getInt32(0));
  elem_idx.push_back(builder.getInt32(index));
  return builder.CreateGEP(fp, elem_idx);
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
      return builder.getInt8PtrTy();

    case TSG_TYPE_PEND:
      assert(false);
      return nullptr;
  }

  assert(false);
  return nullptr;
}

llvm::FunctionType* Compiler::convFuncTy(tsg_type_t* type) {
  assert(type != nullptr && type->kind == TSG_TYPE_FUNC);

  auto ret_type = convTy(type->func.ret);
  auto param_types = std::vector<llvm::Type*>();
  param_types.push_back(builder.getInt8PtrTy());
  convTyArr(param_types, type->func.params);
  auto func_type = llvm::FunctionType::get(ret_type, param_types, false);

  return func_type;
}

llvm::StructType* Compiler::convFrameTy(tsg_frame_t* frame) {
  std::vector<llvm::Type*> member_types;
  if (frame->outer != nullptr) {
    member_types.push_back(convFrameTy(frame->outer)->getPointerTo());
  } else {
    member_types.push_back(builder.getInt8PtrTy());
  }

  tsg_member_node_t* node = frame->head;
  while (node != nullptr) {
    member_types.push_back(convTy(tsg_tyenv_get(tyenv, node->member->tyvar)));
    node = node->next;
  }

  return llvm::StructType::get(builder.getContext(), member_types);
}

void Compiler::convTyArr(std::vector<llvm::Type*>& types, tsg_type_arr_t* arr) {
  tsg_type_t** p = arr->elem;
  tsg_type_t** end = arr->elem + arr->size;

  while (p < end) {
    types.push_back(convTy(*p));
    p++;
  }
}

void Compiler::buildAst(tsg_ast_t* ast, tsg_tyenv_t* env) {
  buildFunc(ast->root, env);
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
  assert(func->tyset == env->tyset);

  auto func_type = tsg_tyenv_get(env, func->ftype);
  auto llvm_func = llvm::Function::Create(
      convFuncTy(func_type), llvm::Function::ExternalLinkage,
      tsg_ident_cstr(func->decl->name), module);

  function_table->set(func, env, llvm_func);
  auto body = llvm::BasicBlock::Create(context, "entry", llvm_func);
  builder.SetInsertPoint(body);

  auto stashed_env = this->tyenv;
  auto stashed_frametype = this->frametype;
  auto stashed_frameptr = this->frameptr;
  this->tyenv = env;
  this->frametype = func->frame;
  this->frameptr = builder.CreateAlloca(convFrameTy(func->frame));
  this->frameptr->setName("$sf");

  auto node = func->params->head;
  int32_t param_index = 0;
  for (auto& arg : llvm_func->args()) {
    if (param_index == 0) {
      arg.setName("$outer");

      if (func->frame->outer != nullptr) {
        auto outer = builder.CreateBitCast(
            &arg, convFrameTy(func->frame->outer)->getPointerTo());
        builder.CreateStore(outer, createObjPtrRaw(frametype->depth, 0));
      } else {
        builder.CreateStore(&arg, createObjPtrRaw(frametype->depth, 0));
      }
    } else {
      arg.setName(tsg_ident_cstr(node->decl->name));
      store(node->decl->object, &arg);
      node = node->next;
    }
    param_index += 1;
  }

  llvm::Value* last_value = buildBlock(func->body);

  if (last_value) {
    builder.CreateRet(last_value);
  } else {
    builder.CreateRetVoid();
  }

  this->frameptr = stashed_frameptr;
  this->frametype = stashed_frametype;
  this->tyenv = stashed_env;

  if (llvm::verifyFunction(*llvm_func, &(llvm::errs()))) {
    llvm::errs() << "verifyFunction Failed\n";
  }

  return llvm_func;
}

llvm::Value* Compiler::buildBlock(tsg_block_t* block) {
  buildFuncList(block->funcs);
  return buildStmtList(block->stmts);
}

void Compiler::buildFuncList(tsg_func_list_t* funcs) {
  auto node = funcs->head;
  while (node != nullptr) {
    auto poly_obj =
        builder.CreateBitCast(this->frameptr, builder.getInt8PtrTy());
    store(node->func->decl->object, poly_obj);
    node = node->next;
  }
}

llvm::Value* Compiler::buildStmtList(tsg_stmt_list_t* stmts) {
  llvm::Value* last_value = nullptr;

  auto node = stmts->head;
  while (node != nullptr) {
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

  assert(false);
  return nullptr;
}

llvm::Value* Compiler::buildStmtVal(tsg_stmt_t* stmt) {
  assert(stmt != nullptr && stmt->kind == TSG_STMT_VAL);

  llvm::Value* value = buildExpr(stmt->val.expr);
  store(stmt->val.decl->object, value);

  return value;
}

llvm::Value* Compiler::buildStmtExpr(tsg_stmt_t* stmt) {
  assert(stmt != nullptr && stmt->kind == TSG_STMT_EXPR);
  return buildExpr(stmt->expr.expr);
}

llvm::Value* Compiler::buildExpr(tsg_expr_t* expr) {
  switch (expr->kind) {
    case TSG_EXPR_BINARY:
      return buildExprBinary(expr);

    case TSG_EXPR_CALL:
      return buildExprCall(expr);

    case TSG_EXPR_IFELSE:
      return buildExprIfelse(expr);

    case TSG_EXPR_IDENT:
      return buildExprIdent(expr);

    case TSG_EXPR_NUMBER:
      return buildExprNumber(expr);
  }

  assert(false);
  return nullptr;
}

llvm::Value* Compiler::buildExprBinary(tsg_expr_t* expr) {
  assert(expr != nullptr && expr->kind == TSG_EXPR_BINARY);

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
      assert(false);
      return nullptr;
  }
}

llvm::Value* Compiler::buildExprCall(tsg_expr_t* expr) {
  assert(expr != nullptr && expr->kind == TSG_EXPR_CALL);

  auto callee_obj = buildExpr(expr->call.callee);
  tsg_type_t* callee_type = tsg_tyenv_get(tyenv, expr->call.callee->tyvar);
  assert(callee_type != NULL && callee_type->kind == TSG_TYPE_POLY);

  std::vector<llvm::Value*> args;
  args.push_back(callee_obj);

  tsg_type_arr_t* arg_types = tsg_type_arr_create(expr->call.args->size);
  tsg_type_t** p = arg_types->elem;

  auto node = expr->call.args->head;
  while (node) {
    auto value = buildExpr(node->expr);
    args.push_back(value);

    tsg_type_t* type = tsg_tyenv_get(tyenv, node->expr->tyvar);
    tsg_type_retain(type);
    *(p++) = type;
    node = node->next;
  }

  auto block = builder.GetInsertBlock();

  tsg_tyenv_t* callee_env = tsg_tymap_get(callee_type->poly.tymap, arg_types);
  llvm::Value* callee_func = fetchFunc(callee_type->poly.func, callee_env);
  tsg_type_arr_destroy(arg_types);

  builder.SetInsertPoint(block);
  return builder.CreateCall(callee_func, args);
}

llvm::Value* Compiler::buildExprIfelse(tsg_expr_t* expr) {
  assert(expr != NULL && expr->kind == TSG_EXPR_IFELSE);

  llvm::Function* func = builder.GetInsertBlock()->getParent();
  auto then_block = llvm::BasicBlock::Create(context, "then");
  auto else_block = llvm::BasicBlock::Create(context, "else");
  auto merge_block = llvm::BasicBlock::Create(context, "merge");

  llvm::Value* cond = buildExpr(expr->ifelse.cond);
  builder.CreateCondBr(cond, then_block, else_block);

  func->getBasicBlockList().push_back(then_block);
  builder.SetInsertPoint(then_block);
  llvm::Value* then_value = buildBlock(expr->ifelse.thn);
  builder.CreateBr(merge_block);
  then_block = builder.GetInsertBlock();

  func->getBasicBlockList().push_back(else_block);
  builder.SetInsertPoint(else_block);
  llvm::Value* else_value = buildBlock(expr->ifelse.els);
  builder.CreateBr(merge_block);
  else_block = builder.GetInsertBlock();

  func->getBasicBlockList().push_back(merge_block);
  builder.SetInsertPoint(merge_block);
  auto phi = builder.CreatePHI(then_value->getType(), 2);
  phi->addIncoming(then_value, then_block);
  phi->addIncoming(else_value, else_block);

  return phi;
}

llvm::Value* Compiler::buildExprIdent(tsg_expr_t* expr) {
  assert(expr != nullptr && expr->kind == TSG_EXPR_IDENT);
  return load(expr->ident.object);
}

llvm::Value* Compiler::buildExprNumber(tsg_expr_t* expr) {
  assert(expr != nullptr && expr->kind == TSG_EXPR_NUMBER);
  return builder.getInt32(expr->number.value);
}

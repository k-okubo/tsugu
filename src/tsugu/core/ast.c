/*--------------------------------------- vi: set ft=c ts=2 sw=2 et: --*-c-*--*/
/**
 * @file ast.c
 *
 ** --------------------------------------------------------------------------*/

#include <tsugu/core/ast.h>

#include <tsugu/core/memory.h>
#include <tsugu/core/platform.h>

static void destroy_stmt_val(tsg_stmt_t* stmt);
static void destroy_stmt_expr(tsg_stmt_t* stmt);

static void destroy_expr_binary(tsg_expr_t* expr);
static void destroy_expr_call(tsg_expr_t* expr);
static void destroy_expr_ifelse(tsg_expr_t* expr);
static void destroy_expr_ident(tsg_expr_t* expr);

tsg_ast_t* tsg_ast_create(void) {
  tsg_ast_t* ast = tsg_malloc_obj(tsg_ast_t);

  ast->root = NULL;
  ast->tyenv = NULL;

  return ast;
}

void tsg_ast_destroy(tsg_ast_t* ast) {
  if (ast == NULL) {
    return;
  }

  tsg_func_destroy(ast->root);
  tsg_tyenv_destroy(ast->tyenv);
  tsg_free(ast);
}

tsg_block_t* tsg_block_create(void) {
  tsg_block_t* block = tsg_malloc_obj(tsg_block_t);

  block->funcs = NULL;
  block->stmts = NULL;

  return block;
}

void tsg_block_destroy(tsg_block_t* block) {
  if (block == NULL) {
    return;
  }

  tsg_func_list_destroy(block->funcs);
  tsg_stmt_list_destroy(block->stmts);
  tsg_free(block);
}

tsg_func_t* tsg_func_create(void) {
  tsg_func_t* func = tsg_malloc_obj(tsg_func_t);

  func->decl = NULL;
  func->tyset = NULL;
  func->frame = NULL;
  func->ftype = NULL;
  func->params = NULL;
  func->body = NULL;

  return func;
}

void tsg_func_destroy(tsg_func_t* func) {
  if (func == NULL) {
    return;
  }

  tsg_decl_destroy(func->decl);
  tsg_tyset_destroy(func->tyset);
  tsg_frame_destroy(func->frame);
  tsg_tyvar_destroy(func->ftype);
  tsg_decl_list_destroy(func->params);
  tsg_block_destroy(func->body);
  tsg_free(func);
}

tsg_stmt_t* tsg_stmt_create(tsg_stmt_kind_t kind) {
  tsg_stmt_t* stmt = tsg_malloc_obj(tsg_stmt_t);

  tsg_memset(stmt, 0, sizeof(tsg_stmt_t));
  stmt->kind = kind;

  return stmt;
}

void tsg_stmt_destroy(tsg_stmt_t* stmt) {
  if (stmt == NULL) {
    return;
  }

  switch (stmt->kind) {
    case TSG_STMT_VAL:
      destroy_stmt_val(stmt);
      break;

    case TSG_STMT_EXPR:
      destroy_stmt_expr(stmt);
      break;
  }

  tsg_free(stmt);
}

void destroy_stmt_val(tsg_stmt_t* stmt) {
  tsg_assert(stmt != NULL && stmt->kind == TSG_STMT_VAL);
  tsg_decl_destroy(stmt->val.decl);
  tsg_expr_destroy(stmt->val.expr);
}

void destroy_stmt_expr(tsg_stmt_t* stmt) {
  tsg_assert(stmt != NULL && stmt->kind == TSG_STMT_EXPR);
  tsg_expr_destroy(stmt->expr.expr);
}

tsg_expr_t* tsg_expr_create(tsg_expr_kind_t kind) {
  tsg_expr_t* expr = tsg_malloc_obj(tsg_expr_t);

  tsg_memset(expr, 0, sizeof(tsg_expr_t));
  expr->kind = kind;

  return expr;
}

void tsg_expr_destroy(tsg_expr_t* expr) {
  if (expr == NULL) {
    return;
  }

  switch (expr->kind) {
    case TSG_EXPR_BINARY:
      destroy_expr_binary(expr);
      break;

    case TSG_EXPR_CALL:
      destroy_expr_call(expr);
      break;

    case TSG_EXPR_IFELSE:
      destroy_expr_ifelse(expr);
      break;

    case TSG_EXPR_IDENT:
      destroy_expr_ident(expr);
      break;

    case TSG_EXPR_NUMBER:
      break;
  }

  tsg_tyvar_destroy(expr->tyvar);
  tsg_free(expr);
}

void destroy_expr_binary(tsg_expr_t* expr) {
  tsg_assert(expr != NULL && expr->kind == TSG_EXPR_BINARY);
  tsg_expr_destroy(expr->binary.lhs);
  tsg_expr_destroy(expr->binary.rhs);
}

void destroy_expr_call(tsg_expr_t* expr) {
  tsg_assert(expr != NULL && expr->kind == TSG_EXPR_CALL);
  tsg_expr_destroy(expr->call.callee);
  tsg_expr_list_destroy(expr->call.args);
}

void destroy_expr_ifelse(tsg_expr_t* expr) {
  tsg_assert(expr != NULL && expr->kind == TSG_EXPR_IFELSE);
  tsg_expr_destroy(expr->ifelse.cond);
  tsg_block_destroy(expr->ifelse.thn);
  tsg_block_destroy(expr->ifelse.els);
}

void destroy_expr_ident(tsg_expr_t* expr) {
  tsg_assert(expr != NULL && expr->kind == TSG_EXPR_IDENT);
  tsg_ident_destroy(expr->ident.name);
  // `expr->ident.object` is a reference
}

tsg_decl_t* tsg_decl_create(void) {
  tsg_decl_t* decl = tsg_malloc_obj(tsg_decl_t);

  decl->name = NULL;
  decl->object = NULL;

  return decl;
}

void tsg_decl_destroy(tsg_decl_t* decl) {
  if (decl == NULL) {
    return;
  }

  tsg_ident_destroy(decl->name);
  // `decl->object` is a reference
  tsg_free(decl);
}

tsg_ident_t* tsg_ident_create(void) {
  tsg_ident_t* ident = tsg_malloc_obj(tsg_ident_t);

  tsg_memset(ident, 0, sizeof(tsg_ident_t));

  return ident;
}

void tsg_ident_destroy(tsg_ident_t* ident) {
  if (ident == NULL) {
    return;
  }

  tsg_free(ident->buffer);
  tsg_free(ident);
}

const char* tsg_ident_cstr(tsg_ident_t* ident) {
  return (const char*)ident->buffer;
}

tsg_func_list_t* tsg_func_list_create(void) {
  tsg_func_list_t* list = tsg_malloc_obj(tsg_func_list_t);

  list->head = NULL;
  list->size = 0;

  return list;
}

tsg_func_node_t* tsg_func_node_create(void) {
  tsg_func_node_t* node = tsg_malloc_obj(tsg_func_node_t);

  node->func = NULL;
  node->next = NULL;

  return node;
}

void tsg_func_list_destroy(tsg_func_list_t* list) {
  if (list == NULL) {
    return;
  }

  tsg_func_node_t* node = list->head;
  while (node != NULL) {
    tsg_func_node_t* next = node->next;
    tsg_func_destroy(node->func);
    tsg_free(node);
    node = next;
  }

  tsg_free(list);
}

tsg_stmt_list_t* tsg_stmt_list_create(void) {
  tsg_stmt_list_t* list = tsg_malloc_obj(tsg_stmt_list_t);

  list->head = NULL;
  list->size = 0;

  return list;
}

tsg_stmt_node_t* tsg_stmt_node_create(void) {
  tsg_stmt_node_t* node = tsg_malloc_obj(tsg_stmt_node_t);

  node->stmt = NULL;
  node->next = NULL;

  return node;
}

void tsg_stmt_list_destroy(tsg_stmt_list_t* list) {
  if (list == NULL) {
    return;
  }

  tsg_stmt_node_t* node = list->head;
  while (node) {
    tsg_stmt_node_t* next = node->next;
    tsg_stmt_destroy(node->stmt);
    tsg_free(node);
    node = next;
  }

  tsg_free(list);
}

tsg_expr_list_t* tsg_expr_list_create(void) {
  tsg_expr_list_t* list = tsg_malloc_obj(tsg_expr_list_t);

  list->head = NULL;
  list->size = 0;

  return list;
}

tsg_expr_node_t* tsg_expr_node_create(void) {
  tsg_expr_node_t* node = tsg_malloc_obj(tsg_expr_node_t);

  node->expr = NULL;
  node->next = NULL;

  return node;
}

void tsg_expr_list_destroy(tsg_expr_list_t* list) {
  if (list == NULL) {
    return;
  }

  tsg_expr_node_t* node = list->head;
  while (node) {
    tsg_expr_node_t* next = node->next;
    tsg_expr_destroy(node->expr);
    tsg_free(node);
    node = next;
  }

  tsg_free(list);
}

tsg_decl_list_t* tsg_decl_list_create(void) {
  tsg_decl_list_t* list = tsg_malloc_obj(tsg_decl_list_t);

  list->head = NULL;
  list->size = 0;

  return list;
}

tsg_decl_node_t* tsg_decl_node_create(void) {
  tsg_decl_node_t* node = tsg_malloc_obj(tsg_decl_node_t);

  node->decl = NULL;
  node->next = NULL;

  return node;
}

void tsg_decl_list_destroy(tsg_decl_list_t* list) {
  if (list == NULL) {
    return;
  }

  tsg_decl_node_t* node = list->head;
  while (node) {
    tsg_decl_node_t* next = node->next;
    tsg_decl_destroy(node->decl);
    tsg_free(node);
    node = next;
  }

  tsg_free(list);
}

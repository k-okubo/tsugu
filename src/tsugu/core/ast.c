/*--------------------------------------- vi: set ft=c ts=2 sw=2 et: --*-c-*--*/
/**
 * @file ast.c
 *
 ** --------------------------------------------------------------------------*/

#include <tsugu/core/ast.h>

#include <tsugu/core/memory.h>

static void destroy_stmt_val(tsg_stmt_t* stmt);
static void destroy_stmt_expr(tsg_stmt_t* stmt);

static void destroy_expr_binary(tsg_expr_t* expr);
static void destroy_expr_call(tsg_expr_t* expr);
static void destroy_expr_ifelse(tsg_expr_t* expr);
static void destroy_expr_variable(tsg_expr_t* expr);

tsg_ast_t* tsg_ast_create(void) {
  tsg_ast_t* ast = tsg_malloc_obj(tsg_ast_t);
  if (ast == NULL) {
    return NULL;
  }

  ast->functions = NULL;
  return ast;
}

void tsg_ast_destroy(tsg_ast_t* ast) {
  if (ast == NULL) {
    return;
  }

  tsg_func_list_destroy(ast->functions);
  tsg_free(ast);
}

tsg_func_t* tsg_func_create(void) {
  tsg_func_t* func = tsg_malloc_obj(tsg_func_t);
  if (func == NULL) {
    return NULL;
  }

  func->decl = NULL;
  func->args = NULL;
  func->body = NULL;
  func->n_types = 0;

  return func;
}

void tsg_func_destroy(tsg_func_t* func) {
  if (func == NULL) {
    return;
  }

  tsg_decl_destroy(func->decl);
  tsg_decl_list_destroy(func->args);
  tsg_block_destroy(func->body);
  tsg_free(func);
}

tsg_block_t* tsg_block_create(void) {
  tsg_block_t* block = tsg_malloc_obj(tsg_block_t);
  if (block == NULL) {
    return NULL;
  }

  block->stmts = NULL;
  block->n_decls = 0;

  return block;
}

void tsg_block_destroy(tsg_block_t* block) {
  if (block == NULL) {
    return;
  }

  tsg_stmt_list_destroy(block->stmts);
  tsg_free(block);
}

tsg_stmt_t* tsg_stmt_create(tsg_stmt_kind_t kind) {
  tsg_stmt_t* stmt = tsg_malloc_obj(tsg_stmt_t);
  if (stmt == NULL) {
    return NULL;
  }

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
  tsg_decl_destroy(stmt->val.decl);
  tsg_expr_destroy(stmt->val.expr);
}

void destroy_stmt_expr(tsg_stmt_t* stmt) {
  tsg_expr_destroy(stmt->expr.expr);
}

tsg_expr_t* tsg_expr_create(tsg_expr_kind_t kind) {
  tsg_expr_t* expr = tsg_malloc_obj(tsg_expr_t);
  if (expr == NULL) {
    return NULL;
  }

  expr->kind = kind;
  expr->type_id = 0;
  expr->loc.begin.line = 0;
  expr->loc.begin.column = 0;
  expr->loc.end.line = 0;
  expr->loc.end.column = 0;

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

    case TSG_EXPR_VARIABLE:
      destroy_expr_variable(expr);
      break;

    case TSG_EXPR_NUMBER:
      break;
  }

  tsg_free(expr);
}

void destroy_expr_binary(tsg_expr_t* expr) {
  tsg_expr_destroy(expr->binary.lhs);
  tsg_expr_destroy(expr->binary.rhs);
}

void destroy_expr_call(tsg_expr_t* expr) {
  tsg_expr_destroy(expr->call.callee);
  tsg_expr_list_destroy(expr->call.args);
}

void destroy_expr_ifelse(tsg_expr_t* expr) {
  tsg_expr_destroy(expr->ifelse.cond);
  tsg_block_destroy(expr->ifelse.thn);
  tsg_block_destroy(expr->ifelse.els);
}

void destroy_expr_variable(tsg_expr_t* expr) {
  tsg_ident_destroy(expr->variable.name);
}

tsg_decl_t* tsg_decl_create(void) {
  tsg_decl_t* decl = tsg_malloc_obj(tsg_decl_t);
  if (decl == NULL) {
    return NULL;
  }

  decl->name = NULL;
  decl->type_id = 0;
  decl->depth = 0;
  decl->index = 0;

  return decl;
}

void tsg_decl_destroy(tsg_decl_t* decl) {
  if (decl == NULL) {
    return;
  }

  tsg_ident_destroy(decl->name);
  tsg_free(decl);
}

tsg_ident_t* tsg_ident_create(void) {
  tsg_ident_t* ident = tsg_malloc_obj(tsg_ident_t);
  if (ident == NULL) {
    return NULL;
  }

  ident->buffer = NULL;
  ident->nbytes = 0;
  ident->loc.begin.line = 0;
  ident->loc.begin.column = 0;
  ident->loc.end.line = 0;
  ident->loc.end.column = 0;

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
  if (list == NULL) {
    return NULL;
  }

  list->head = NULL;
  list->size = 0;

  return list;
}

tsg_func_node_t* tsg_func_node_create(void) {
  tsg_func_node_t* node = tsg_malloc_obj(tsg_func_node_t);
  if (node == NULL) {
    return NULL;
  }

  node->func = NULL;
  node->next = NULL;

  return node;
}

void tsg_func_list_destroy(tsg_func_list_t* list) {
  if (list == NULL) {
    return;
  }

  tsg_func_node_t* node = list->head;
  while (node) {
    tsg_func_node_t* next = node->next;
    tsg_func_destroy(node->func);
    tsg_free(node);
    node = next;
  }
  tsg_free(list);
}

tsg_stmt_list_t* tsg_stmt_list_create(void) {
  tsg_stmt_list_t* list = tsg_malloc_obj(tsg_stmt_list_t);
  if (list == NULL) {
    return NULL;
  }

  list->head = NULL;
  list->size = 0;

  return list;
}

tsg_stmt_node_t* tsg_stmt_node_create(void) {
  tsg_stmt_node_t* node = tsg_malloc_obj(tsg_stmt_node_t);
  if (node == NULL) {
    return NULL;
  }

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
  if (list == NULL) {
    return NULL;
  }

  list->head = NULL;
  list->size = 0;

  return list;
}

tsg_expr_node_t* tsg_expr_node_create(void) {
  tsg_expr_node_t* node = tsg_malloc_obj(tsg_expr_node_t);
  if (node == NULL) {
    return NULL;
  }

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
  if (list == NULL) {
    return NULL;
  }

  list->head = NULL;
  list->size = 0;

  return list;
}

tsg_decl_node_t* tsg_decl_node_create(void) {
  tsg_decl_node_t* node = tsg_malloc_obj(tsg_decl_node_t);
  if (node == NULL) {
    return NULL;
  }

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

/*--------------------------------------- vi: set ft=c ts=2 sw=2 et: --*-c-*--*/
/**
 * @file ast.c
 *
 ** --------------------------------------------------------------------------*/

#include <tsugu/core/ast.h>

#include <tsugu/core/memory.h>

static void destroy_ast(tsg_ast_t* ast);
static void destroy_func(tsg_func_t* func);
static void destroy_block(tsg_block_t* block);

static void destroy_stmt(tsg_stmt_t* stmt);
static void destroy_stmt_val(tsg_stmt_t* stmt);
static void destroy_stmt_expr(tsg_stmt_t* stmt);

static void destroy_expr(tsg_expr_t* expr);
static void destroy_expr_binary(tsg_expr_t* expr);
static void destroy_expr_call(tsg_expr_t* expr);
static void destroy_expr_ifelse(tsg_expr_t* expr);
static void destroy_expr_variable(tsg_expr_t* expr);

static void destroy_decl(tsg_decl_t* decl);
static void destroy_ident(tsg_ident_t* ident);

static void destroy_func_list(tsg_func_list_t* list);
static void destroy_stmt_list(tsg_stmt_list_t* list);
static void destroy_expr_list(tsg_expr_list_t* list);
static void destroy_decl_list(tsg_decl_list_t* list);

const char* tsg_ident_cstr(tsg_ident_t* ident) {
  return (const char*)ident->buffer;
}

void tsg_ast_destroy(tsg_ast_t* ast) {
  destroy_ast(ast);
}

void destroy_ast(tsg_ast_t* ast) {
  if (ast == NULL) {
    return;
  }

  destroy_func_list(ast->functions);
  tsg_free(ast);
}

void destroy_func(tsg_func_t* func) {
  if (func == NULL) {
    return;
  }

  destroy_decl(func->decl);
  destroy_decl_list(func->args);
  destroy_block(func->body);
  tsg_free(func);
}

void destroy_block(tsg_block_t* block) {
  if (block == NULL) {
    return;
  }

  destroy_stmt_list(block->stmts);
  tsg_free(block);
}

void destroy_stmt(tsg_stmt_t* stmt) {
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
  destroy_decl(stmt->val.decl);
  destroy_expr(stmt->val.expr);
}

void destroy_stmt_expr(tsg_stmt_t* stmt) {
  destroy_expr(stmt->expr.expr);
}

void destroy_expr(tsg_expr_t* expr) {
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
  destroy_expr(expr->binary.lhs);
  destroy_expr(expr->binary.rhs);
}

void destroy_expr_call(tsg_expr_t* expr) {
  destroy_expr(expr->call.callee);
  destroy_expr_list(expr->call.args);
}

void destroy_expr_ifelse(tsg_expr_t* expr) {
  destroy_expr(expr->ifelse.cond);
  destroy_block(expr->ifelse.thn);
  destroy_block(expr->ifelse.els);
}

void destroy_expr_variable(tsg_expr_t* expr) {
  destroy_ident(expr->variable.name);
}

void destroy_decl(tsg_decl_t* decl) {
  if (decl == NULL) {
    return;
  }

  destroy_ident(decl->name);
  tsg_free(decl);
}

void destroy_ident(tsg_ident_t* ident) {
  if (ident == NULL) {
    return;
  }

  tsg_free(ident->buffer);
  tsg_free(ident);
}

void destroy_func_list(tsg_func_list_t* list) {
  if (list == NULL) {
    return;
  }

  tsg_func_node_t* node = list->head;
  while (node) {
    tsg_func_node_t* next = node->next;
    destroy_func(node->func);
    tsg_free(node);
    node = next;
  }
  tsg_free(list);
}

void destroy_stmt_list(tsg_stmt_list_t* list) {
  if (list == NULL) {
    return;
  }

  tsg_stmt_node_t* node = list->head;
  while (node) {
    tsg_stmt_node_t* next = node->next;
    destroy_stmt(node->stmt);
    tsg_free(node);
    node = next;
  }
  tsg_free(list);
}

void destroy_expr_list(tsg_expr_list_t* list) {
  if (list == NULL) {
    return;
  }

  tsg_expr_node_t* node = list->head;
  while (node) {
    tsg_expr_node_t* next = node->next;
    destroy_expr(node->expr);
    tsg_free(node);
    node = next;
  }
  tsg_free(list);
}

void destroy_decl_list(tsg_decl_list_t* list) {
  if (list == NULL) {
    return;
  }

  tsg_decl_node_t* node = list->head;
  while (node) {
    tsg_decl_node_t* next = node->next;
    destroy_decl(node->decl);
    tsg_free(node);
    node = next;
  }
  tsg_free(list);
}

/*--------------------------------------- vi: set ft=c ts=2 sw=2 et: --*-c-*--*/
/**
 * @file resolver.c
 *
 ** --------------------------------------------------------------------------*/

#include <tsugu/core/resolver.h>

#include <tsugu/core/memory.h>
#include <tsugu/core/scope.h>

struct tsg_resolver_s {
  tsg_errlist_t errors;
  tsg_tyset_t* tyset;
  tsg_frame_t* frame;
  tsg_scope_t* scope;
};

static tsg_tyset_t* open_tyset(tsg_resolver_t* resolver);
static void close_tyset(tsg_resolver_t* resolver, tsg_tyset_t* outer);
static tsg_frame_t* open_frame(tsg_resolver_t* resolver);
static void close_frame(tsg_resolver_t* resolver, tsg_frame_t* outer);
static tsg_scope_t* open_scope(tsg_resolver_t* resolver);
static void close_scope(tsg_resolver_t* resolver, tsg_scope_t* outer);
static bool declare(tsg_resolver_t* resolver, tsg_decl_t* decl);
static tsg_member_t* lookup(tsg_resolver_t* resolver, tsg_ident_t* name);
static void error(tsg_resolver_t* resolver, tsg_source_range_t* loc,
                  const char* format, ...);

static void resolve_ast(tsg_resolver_t* resolver, tsg_ast_t* ast);
static void resolve_func_proto(tsg_resolver_t* resolver, tsg_func_t* func);
static void resolve_func_body(tsg_resolver_t* resolver, tsg_func_t* func);
static void resolve_block(tsg_resolver_t* resolver, tsg_block_t* block);
static void resolve_func_list(tsg_resolver_t* resolver, tsg_func_list_t* list);
static void resolve_stmt_list(tsg_resolver_t* resolver, tsg_stmt_list_t* list);

static void resolve_stmt(tsg_resolver_t* resolver, tsg_stmt_t* stmt);
static void resolve_stmt_val(tsg_resolver_t* resolver, tsg_stmt_t* stmt);
static void resolve_stmt_expr(tsg_resolver_t* resolver, tsg_stmt_t* stmt);

static void resolve_expr(tsg_resolver_t* resolver, tsg_expr_t* expr);
static void resolve_expr_binary(tsg_resolver_t* resolver, tsg_expr_t* expr);
static void resolve_expr_call(tsg_resolver_t* resolver, tsg_expr_t* expr);
static void resolve_expr_ifelse(tsg_resolver_t* resolver, tsg_expr_t* expr);
static void resolve_expr_ident(tsg_resolver_t* resolver, tsg_expr_t* expr);

static void resolve_expr_list(tsg_resolver_t* resolver, tsg_expr_list_t* list);

tsg_resolver_t* tsg_resolver_create(void) {
  tsg_resolver_t* resolver = tsg_malloc_obj(tsg_resolver_t);
  if (resolver == NULL) {
    return NULL;
  }

  tsg_errlist_init(&(resolver->errors));
  resolver->tyset = NULL;
  resolver->frame = NULL;
  resolver->scope = NULL;

  return resolver;
}

void tsg_resolver_destroy(tsg_resolver_t* resolver) {
  tsg_errlist_release(&(resolver->errors));
  tsg_assert(resolver->tyset == NULL);
  tsg_assert(resolver->frame == NULL);
  tsg_assert(resolver->scope == NULL);
  tsg_free(resolver);
}

void tsg_resolver_error(const tsg_resolver_t* resolver, tsg_errlist_t* errors) {
  *errors = resolver->errors;
}

tsg_tyset_t* open_tyset(tsg_resolver_t* resolver) {
  tsg_tyset_t* outer = resolver->tyset;
  resolver->tyset = tsg_tyset_create(outer);
  return outer;
}

void close_tyset(tsg_resolver_t* resolver, tsg_tyset_t* outer) {
  tsg_assert(resolver->tyset != NULL);
  resolver->tyset = outer;
}

tsg_frame_t* open_frame(tsg_resolver_t* resolver) {
  tsg_frame_t* outer = resolver->frame;
  resolver->frame = tsg_frame_create(outer);
  return outer;
}

void close_frame(tsg_resolver_t* resolver, tsg_frame_t* outer) {
  tsg_assert(resolver->frame != NULL);
  resolver->frame = outer;
}

tsg_scope_t* open_scope(tsg_resolver_t* resolver) {
  tsg_scope_t* outer = resolver->scope;
  resolver->scope = tsg_scope_create(outer);
  return outer;
}

void close_scope(tsg_resolver_t* resolver, tsg_scope_t* outer) {
  tsg_assert(resolver->scope != NULL);
  tsg_scope_destroy(resolver->scope);
  resolver->scope = outer;
}

bool declare(tsg_resolver_t* resolver, tsg_decl_t* decl) {
  tsg_assert(decl != NULL);
  tsg_assert(decl->name != NULL);
  tsg_assert(decl->object == NULL);

  decl->object = tsg_frame_add_member(resolver->frame);
  decl->object->tyvar = tsg_tyvar_create(resolver->tyset);

  if (tsg_scope_add(resolver->scope, decl->name, decl->object) == false) {
    error(resolver, &(decl->name->loc), "redefinition '%I'", decl->name);
    return false;
  } else {
    return true;
  }
}

tsg_member_t* lookup(tsg_resolver_t* resolver, tsg_ident_t* name) {
  tsg_assert(name != NULL);
  tsg_assert(name->nbytes > 0);

  tsg_member_t* object = tsg_scope_find(resolver->scope, name);

  if (object == NULL) {
    error(resolver, &(name->loc), "undeclared '%I'", name);
    return NULL;
  } else {
    return object;
  }
}

void error(tsg_resolver_t* resolver, tsg_source_range_t* loc,
           const char* format, ...) {
  va_list args;
  va_start(args, format);
  tsg_errorv(&(resolver->errors), loc, format, args);
  va_end(args);
}

bool tsg_resolver_resolve(tsg_resolver_t* resolver, tsg_ast_t* ast) {
  resolve_ast(resolver, ast);
  return resolver->errors.head == NULL;
}

void resolve_ast(tsg_resolver_t* resolver, tsg_ast_t* ast) {
  resolve_func_body(resolver, ast->root);
}

void resolve_func_proto(tsg_resolver_t* resolver, tsg_func_t* func) {
  declare(resolver, func->decl);
}

void resolve_func_body(tsg_resolver_t* resolver, tsg_func_t* func) {
  tsg_tyset_t* outer_tyset = open_tyset(resolver);
  tsg_frame_t* outer_frame = open_frame(resolver);
  tsg_scope_t* outer_scope = open_scope(resolver);

  func->tyset = resolver->tyset;
  func->frame = resolver->frame;
  func->ftype = tsg_tyvar_create(resolver->tyset);

  tsg_decl_node_t* node = func->params->head;
  while (node) {
    declare(resolver, node->decl);
    node = node->next;
  }

  resolve_block(resolver, func->body);

  close_scope(resolver, outer_scope);
  close_frame(resolver, outer_frame);
  close_tyset(resolver, outer_tyset);
}

void resolve_block(tsg_resolver_t* resolver, tsg_block_t* block) {
  resolve_func_list(resolver, block->funcs);
  resolve_stmt_list(resolver, block->stmts);
}

void resolve_func_list(tsg_resolver_t* resolver, tsg_func_list_t* list) {
  tsg_func_node_t* node = list->head;
  while (node != NULL) {
    resolve_func_proto(resolver, node->func);
    node = node->next;
  }

  node = list->head;
  while (node != NULL) {
    resolve_func_body(resolver, node->func);
    node = node->next;
  }
}

void resolve_stmt_list(tsg_resolver_t* resolver, tsg_stmt_list_t* list) {
  tsg_stmt_node_t* node = list->head;
  while (node != NULL) {
    resolve_stmt(resolver, node->stmt);
    node = node->next;
  }
}

void resolve_stmt(tsg_resolver_t* resolver, tsg_stmt_t* stmt) {
  switch (stmt->kind) {
    case TSG_STMT_VAL:
      resolve_stmt_val(resolver, stmt);
      break;

    case TSG_STMT_EXPR:
      resolve_stmt_expr(resolver, stmt);
      break;
  }
}

void resolve_stmt_val(tsg_resolver_t* resolver, tsg_stmt_t* stmt) {
  tsg_assert(stmt != NULL && stmt->kind == TSG_STMT_VAL);
  resolve_expr(resolver, stmt->val.expr);
  declare(resolver, stmt->val.decl);
}

void resolve_stmt_expr(tsg_resolver_t* resolver, tsg_stmt_t* stmt) {
  tsg_assert(stmt != NULL && stmt->kind == TSG_STMT_EXPR);
  resolve_expr(resolver, stmt->expr.expr);
}

void resolve_expr(tsg_resolver_t* resolver, tsg_expr_t* expr) {
  switch (expr->kind) {
    case TSG_EXPR_BINARY:
      resolve_expr_binary(resolver, expr);
      break;

    case TSG_EXPR_CALL:
      resolve_expr_call(resolver, expr);
      break;

    case TSG_EXPR_IFELSE:
      resolve_expr_ifelse(resolver, expr);
      break;

    case TSG_EXPR_IDENT:
      resolve_expr_ident(resolver, expr);
      break;

    case TSG_EXPR_NUMBER:
      break;
  }

  expr->tyvar = tsg_tyvar_create(resolver->tyset);
}

void resolve_expr_binary(tsg_resolver_t* resolver, tsg_expr_t* expr) {
  tsg_assert(expr != NULL && expr->kind == TSG_EXPR_BINARY);
  resolve_expr(resolver, expr->binary.lhs);
  resolve_expr(resolver, expr->binary.rhs);
}

void resolve_expr_call(tsg_resolver_t* resolver, tsg_expr_t* expr) {
  tsg_assert(expr != NULL && expr->kind == TSG_EXPR_CALL);
  resolve_expr(resolver, expr->call.callee);
  resolve_expr_list(resolver, expr->call.args);
}

void resolve_expr_ifelse(tsg_resolver_t* resolver, tsg_expr_t* expr) {
  tsg_assert(expr != NULL && expr->kind == TSG_EXPR_IFELSE);

  resolve_expr(resolver, expr->ifelse.cond);
  tsg_scope_t* outer_scope;

  outer_scope = open_scope(resolver);
  resolve_block(resolver, expr->ifelse.thn);
  close_scope(resolver, outer_scope);

  outer_scope = open_scope(resolver);
  resolve_block(resolver, expr->ifelse.els);
  close_scope(resolver, outer_scope);
}

void resolve_expr_ident(tsg_resolver_t* resolver, tsg_expr_t* expr) {
  tsg_assert(expr != NULL && expr->kind == TSG_EXPR_IDENT);
  tsg_member_t* object = lookup(resolver, expr->ident.name);
  expr->ident.object = object;
}

void resolve_expr_list(tsg_resolver_t* resolver, tsg_expr_list_t* list) {
  tsg_expr_node_t* node = list->head;
  while (node) {
    resolve_expr(resolver, node->expr);
    node = node->next;
  }
}

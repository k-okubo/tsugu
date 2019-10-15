/*--------------------------------------- vi: set ft=c ts=2 sw=2 et: --*-c-*--*/
/**
 * @file resolver.c
 *
 ** --------------------------------------------------------------------------*/

#include <tsugu/core/resolver.h>

#include <tsugu/core/memory.h>
#include <tsugu/core/symtbl.h>

typedef struct scope_s scope_t;
struct scope_s {
  tsg_symtbl_t* symtbl;
  int32_t depth;
  int32_t size;
  scope_t* outer;
};

struct tsg_resolver_s {
  scope_t* scope;
  tsg_errlist_t errors;
  scope_t* func_scope;
};

static void enter_scope(tsg_resolver_t* resolver);
static void leave_scope(tsg_resolver_t* resolver);
static bool insert(tsg_resolver_t* resolver, tsg_decl_t* decl);
static tsg_decl_t* lookup(tsg_resolver_t* resolver, tsg_ident_t* name);
static void error(tsg_resolver_t* resolver, tsg_source_range_t* loc,
                  const char* format, ...);

static void resolve_ast(tsg_resolver_t* resolver, tsg_ast_t* ast);
static void resolve_func_proto(tsg_resolver_t* resolver, tsg_func_t* func);
static void resolve_func_body(tsg_resolver_t* resolver, tsg_func_t* func);
static void resolve_block(tsg_resolver_t* resolver, tsg_block_t* block);

static void resolve_stmt(tsg_resolver_t* resolver, tsg_stmt_t* stmt);
static void resolve_stmt_val(tsg_resolver_t* resolver, tsg_stmt_t* stmt);
static void resolve_stmt_expr(tsg_resolver_t* resolver, tsg_stmt_t* stmt);

static void resolve_expr(tsg_resolver_t* resolver, tsg_expr_t* expr);
static void resolve_expr_binary(tsg_resolver_t* resolver, tsg_expr_t* expr);
static void resolve_expr_call(tsg_resolver_t* resolver, tsg_expr_t* expr);
static void resolve_expr_ifelse(tsg_resolver_t* resolver, tsg_expr_t* expr);
static void resolve_expr_variable(tsg_resolver_t* resolver, tsg_expr_t* expr);

static void resolve_expr_list(tsg_resolver_t* resolver, tsg_expr_list_t* list);

tsg_resolver_t* tsg_resolver_create(void) {
  tsg_resolver_t* resolver = tsg_malloc_obj(tsg_resolver_t);
  if (resolver == NULL) {
    return NULL;
  }

  resolver->scope = NULL;
  tsg_errlist_init(&(resolver->errors));
  resolver->func_scope = NULL;

  return resolver;
}

void tsg_resolver_destroy(tsg_resolver_t* resolver) {
  scope_t* scope = resolver->scope;
  while (scope) {
    scope_t* next = scope->outer;
    tsg_symtbl_destroy(scope->symtbl);
    tsg_free(scope);
    scope = next;
  }

  tsg_errlist_release(&(resolver->errors));
  tsg_free(resolver);
}

void tsg_resolver_error(const tsg_resolver_t* resolver, tsg_errlist_t* errors) {
  *errors = resolver->errors;
}

void enter_scope(tsg_resolver_t* resolver) {
  int32_t depth = 0;
  if (resolver->scope) {
    depth = resolver->scope->depth + 1;
  }

  scope_t* scope = tsg_malloc_obj(scope_t);
  scope->symtbl = tsg_symtbl_create();
  scope->depth = depth;
  scope->size = 0;
  scope->outer = resolver->scope;

  resolver->scope = scope;
}

void leave_scope(tsg_resolver_t* resolver) {
  scope_t* scope = resolver->scope;
  resolver->scope = scope->outer;

  tsg_symtbl_destroy(scope->symtbl);
  tsg_free(scope);
}

bool insert(tsg_resolver_t* resolver, tsg_decl_t* decl) {
  scope_t* scope = resolver->scope;

  if (tsg_symtbl_insert(scope->symtbl, decl) == false) {
    error(resolver, &(decl->name->loc), "redefinition '%I'", decl->name);
    return false;
  } else {
    decl->depth = scope->depth;
    decl->index = scope->size;
    scope->size += 1;
    return true;
  }
}

tsg_decl_t* lookup(tsg_resolver_t* resolver, tsg_ident_t* name) {
  scope_t* scope = resolver->scope;
  while (scope) {
    tsg_decl_t* ret = NULL;
    if (tsg_symtbl_lookup(scope->symtbl, name, &ret)) {
      return ret;
    }
    scope = scope->outer;
  }

  error(resolver, &(name->loc), "undeclared '%I'", name);
  return NULL;
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
  enter_scope(resolver);

  tsg_func_node_t* node = ast->functions->head;
  while (node) {
    resolve_func_proto(resolver, node->func);
    node = node->next;
  }

  node = ast->functions->head;
  while (node) {
    resolve_func_body(resolver, node->func);
    node = node->next;
  }

  leave_scope(resolver);
}

void resolve_func_proto(tsg_resolver_t* resolver, tsg_func_t* func) {
  insert(resolver, func->decl);
}

void resolve_func_body(tsg_resolver_t* resolver, tsg_func_t* func) {
  enter_scope(resolver);

  tsg_decl_node_t* node = func->args->head;
  while (node) {
    insert(resolver, node->decl);
    node = node->next;
  }

  resolve_block(resolver, func->body);
  leave_scope(resolver);
}

void resolve_block(tsg_resolver_t* resolver, tsg_block_t* block) {
  tsg_stmt_node_t* node = block->stmts->head;
  while (node) {
    resolve_stmt(resolver, node->stmt);
    node = node->next;
  }

  block->n_decls = resolver->scope->size;
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
  resolve_expr(resolver, stmt->val.expr);
  insert(resolver, stmt->val.decl);
}

void resolve_stmt_expr(tsg_resolver_t* resolver, tsg_stmt_t* stmt) {
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

    case TSG_EXPR_VARIABLE:
      resolve_expr_variable(resolver, expr);
      break;

    case TSG_EXPR_NUMBER:
      break;
  }
}

void resolve_expr_binary(tsg_resolver_t* resolver, tsg_expr_t* expr) {
  resolve_expr(resolver, expr->binary.lhs);
  resolve_expr(resolver, expr->binary.rhs);
}

void resolve_expr_call(tsg_resolver_t* resolver, tsg_expr_t* expr) {
  resolve_expr(resolver, expr->call.callee);
  resolve_expr_list(resolver, expr->call.args);
}

void resolve_expr_ifelse(tsg_resolver_t* resolver, tsg_expr_t* expr) {
  resolve_expr(resolver, expr->ifelse.cond);

  enter_scope(resolver);
  resolve_block(resolver, expr->ifelse.thn);
  leave_scope(resolver);

  enter_scope(resolver);
  resolve_block(resolver, expr->ifelse.els);
  leave_scope(resolver);
}

void resolve_expr_variable(tsg_resolver_t* resolver, tsg_expr_t* expr) {
  tsg_decl_t* decl = lookup(resolver, expr->variable.name);
  expr->variable.resolved = decl;
}

void resolve_expr_list(tsg_resolver_t* resolver, tsg_expr_list_t* list) {
  tsg_expr_node_t* node = list->head;
  while (node) {
    resolve_expr(resolver, node->expr);
    node = node->next;
  }
}

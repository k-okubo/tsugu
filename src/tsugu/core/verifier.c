/*--------------------------------------- vi: set ft=c ts=2 sw=2 et: --*-c-*--*/
/**
 * @file verifier.c
 *
 ** --------------------------------------------------------------------------*/

#include <tsugu/core/verifier.h>

#include <tsugu/core/memory.h>
#include <tsugu/core/tyenv.h>
#include <tsugu/core/tymap.h>
#include <tsugu/core/type.h>

struct tsg_verifier_s {
  tsg_tyenv_t* root_env;
  tsg_tyenv_t* func_env;
  tsg_errlist_t errors;
};

static void error(tsg_verifier_t* verifier, tsg_source_range_t* loc,
                  const char* format, ...);

static void verify_ast(tsg_verifier_t* verifier, tsg_ast_t* ast);
static void verify_func(tsg_verifier_t* verifier, tsg_func_t* func,
                        tsg_type_arr_t* arg_types);
static tsg_type_t* verify_block(tsg_verifier_t* verifier, tsg_block_t* block);

static tsg_type_t* verify_stmt(tsg_verifier_t* verifier, tsg_stmt_t* stmt);
static tsg_type_t* verify_stmt_val(tsg_verifier_t* verifier, tsg_stmt_t* stmt);
static tsg_type_t* verify_stmt_expr(tsg_verifier_t* verifier, tsg_stmt_t* stmt);

static tsg_type_t* verify_expr(tsg_verifier_t* verifier, tsg_expr_t* expr);
static tsg_type_t* verify_expr_binary(tsg_verifier_t* verifier,
                                      tsg_expr_t* expr);
static tsg_type_t* verify_expr_call(tsg_verifier_t* verifier, tsg_expr_t* expr);
static tsg_type_t* verify_expr_ifelse(tsg_verifier_t* verifier,
                                      tsg_expr_t* expr);
static tsg_type_t* verify_expr_variable(tsg_verifier_t* verifier,
                                        tsg_expr_t* expr);
static tsg_type_t* verify_expr_number(tsg_verifier_t* verifier,
                                      tsg_expr_t* expr);

static tsg_type_arr_t* verify_expr_list(tsg_verifier_t* verifier,
                                        tsg_expr_list_t* list);

tsg_verifier_t* tsg_verifier_create(void) {
  tsg_verifier_t* verifier = tsg_malloc_obj(tsg_verifier_t);
  if (verifier == NULL) {
    return NULL;
  }

  verifier->root_env = NULL;
  verifier->func_env = NULL;
  tsg_errlist_init(&(verifier->errors));

  return verifier;
}

void tsg_verifier_destroy(tsg_verifier_t* verifier) {
  tsg_errlist_release(&(verifier->errors));
  tsg_free(verifier);
}

void tsg_verifier_error(const tsg_verifier_t* verifier, tsg_errlist_t* errors) {
  *errors = verifier->errors;
}

void error(tsg_verifier_t* verifier, tsg_source_range_t* loc,
           const char* format, ...) {
  va_list args;
  va_start(args, format);
  tsg_errorv(&(verifier->errors), loc, format, args);
  va_end(args);
}

bool tsg_verifier_verify(tsg_verifier_t* verifier, tsg_ast_t* ast,
                         tsg_tyenv_t** root_env) {
  *root_env = tsg_tyenv_create(NULL, ast->functions->size);
  verifier->root_env = *root_env;
  verify_ast(verifier, ast);
  return verifier->errors.head == NULL;
}

void verify_ast(tsg_verifier_t* verifier, tsg_ast_t* ast) {
  tsg_func_t* main_func = NULL;
  tsg_type_t* main_type = NULL;

  tsg_func_node_t* node = ast->functions->head;
  while (node) {
    tsg_type_t* type = tsg_type_create();
    type->kind = TSG_TYPE_POLY;
    type->poly.func = node->func;
    type->poly.tymap = tsg_tymap_create();
    tsg_tyenv_set(verifier->root_env, node->func->decl->type_id, type);

    if (tsg_memcmp(node->func->decl->name->buffer, "main", 4) == 0) {
      main_func = node->func;
      main_type = type;
    }

    node = node->next;
  }

  verifier->func_env = tsg_tyenv_create(verifier->root_env, main_func->n_types);
  tsg_type_arr_t* main_args = tsg_type_arr_create(0);
  tsg_tymap_add(main_type->poly.tymap, main_args, verifier->func_env);

  verify_func(verifier, main_func, main_args);
}

void verify_func(tsg_verifier_t* verifier, tsg_func_t* func,
                 tsg_type_arr_t* arg_types) {
  if (func->args->size != arg_types->size) {
    return;
  }

  tsg_type_t* func_type = tsg_type_create();
  func_type->kind = TSG_TYPE_FUNC;
  func_type->func.params = arg_types;
  func_type->func.ret = tsg_type_create();
  func_type->func.ret->kind = TSG_TYPE_PEND;
  tsg_tyenv_set(verifier->func_env, 0, func_type);

  tsg_decl_node_t* decl = func->args->head;
  tsg_type_t** ptype = arg_types->elem;
  while (decl) {
    tsg_type_t* type = *ptype;
    tsg_type_retain(type);
    tsg_tyenv_set(verifier->func_env, decl->decl->type_id, type);

    decl = decl->next;
    ptype++;
  }

  tsg_type_t* ret_type = verify_block(verifier, func->body);
  tsg_type_release(func_type->func.ret);
  func_type->func.ret = ret_type;
}

tsg_type_t* verify_block(tsg_verifier_t* verifier, tsg_block_t* block) {
  tsg_type_t* last_stmt_type = NULL;

  tsg_stmt_node_t* node = block->stmts->head;
  while (node) {
    if (last_stmt_type != NULL) {
      tsg_type_release(last_stmt_type);
    }

    last_stmt_type = verify_stmt(verifier, node->stmt);
    node = node->next;
  }

  return last_stmt_type;
}

tsg_type_t* verify_stmt(tsg_verifier_t* verifier, tsg_stmt_t* stmt) {
  switch (stmt->kind) {
    case TSG_STMT_VAL:
      return verify_stmt_val(verifier, stmt);

    case TSG_STMT_EXPR:
      return verify_stmt_expr(verifier, stmt);
  }

  return NULL;
}

tsg_type_t* verify_stmt_val(tsg_verifier_t* verifier, tsg_stmt_t* stmt) {
  tsg_type_t* type = verify_expr(verifier, stmt->val.expr);
  tsg_tyenv_set(verifier->func_env, stmt->val.decl->type_id, type);

  tsg_type_retain(type);
  return type;
}

tsg_type_t* verify_stmt_expr(tsg_verifier_t* verifier, tsg_stmt_t* stmt) {
  return verify_expr(verifier, stmt->expr.expr);
}

tsg_type_t* verify_expr(tsg_verifier_t* verifier, tsg_expr_t* expr) {
  switch (expr->kind) {
    case TSG_EXPR_BINARY:
      return verify_expr_binary(verifier, expr);

    case TSG_EXPR_CALL:
      return verify_expr_call(verifier, expr);

    case TSG_EXPR_IFELSE:
      return verify_expr_ifelse(verifier, expr);

    case TSG_EXPR_VARIABLE:
      return verify_expr_variable(verifier, expr);

    case TSG_EXPR_NUMBER:
      return verify_expr_number(verifier, expr);
  }

  return NULL;
}

tsg_type_t* verify_expr_binary(tsg_verifier_t* verifier, tsg_expr_t* expr) {
  tsg_type_t* lhs_type = verify_expr(verifier, expr->binary.lhs);
  tsg_type_t* rhs_type = verify_expr(verifier, expr->binary.rhs);

  if (lhs_type == NULL || rhs_type == NULL) {
    return NULL;
  }

  tsg_type_t* type = tsg_type_create();

  if (lhs_type->kind == rhs_type->kind) {
    type->kind = lhs_type->kind;
  } else if (lhs_type->kind == TSG_TYPE_PEND) {
    type->kind = rhs_type->kind;
  } else if (rhs_type->kind == TSG_TYPE_PEND) {
    type->kind = lhs_type->kind;
  } else {
    error(verifier, &(expr->loc), "incompatible type");
    tsg_type_release(type);
    type = NULL;
  }

  if (type) {
    switch (expr->binary.op) {
      case TSG_TOKEN_EQ:
      case TSG_TOKEN_LT:
      case TSG_TOKEN_GT:
        type->kind = TSG_TYPE_BOOL;
        break;

      default:
        break;
    }
  }

  tsg_type_release(lhs_type);
  tsg_type_release(rhs_type);

  tsg_tyenv_set(verifier->func_env, expr->type_id, type);
  return type;
}

tsg_type_t* verify_expr_call(tsg_verifier_t* verifier, tsg_expr_t* expr) {
  tsg_type_t* callee_type = verify_expr(verifier, expr->call.callee);
  tsg_type_arr_t* arg_types = verify_expr_list(verifier, expr->call.args);

  if (callee_type == NULL) {
    return NULL;
  }

  if (callee_type->kind != TSG_TYPE_POLY) {
    error(verifier, &(expr->call.callee->loc), "callee is not a function");
    return NULL;
  }

  tsg_func_t* func = callee_type->poly.func;
  tsg_type_t* func_type = NULL;

  tsg_tyenv_t* func_env = tsg_tymap_get(callee_type->poly.tymap, arg_types);

  if (func_env != NULL) {
    // already typed
    func_type = tsg_tyenv_get(func_env, 0);
  } else {
    if (expr->call.args->size < callee_type->poly.func->args->size) {
      error(verifier, &(expr->loc), "too few arguments");
      return NULL;
    }
    if (expr->call.args->size > callee_type->poly.func->args->size) {
      error(verifier, &(expr->loc), "too many arguments");
      return NULL;
    }

    // create specific typed function instance
    func_env = tsg_tyenv_create(verifier->root_env, func->n_types);
    tsg_tymap_add(callee_type->poly.tymap, arg_types, func_env);

    tsg_tyenv_t* prev_env = verifier->func_env;
    verifier->func_env = func_env;

    verify_func(verifier, func, arg_types);
    func_type = tsg_tyenv_get(func_env, 0);

    verifier->func_env = prev_env;
  }

  tsg_type_t* type = func_type->func.ret;
  tsg_type_retain(type);
  tsg_tyenv_set(verifier->func_env, expr->type_id, type);
  return type;
}

tsg_type_t* verify_expr_ifelse(tsg_verifier_t* verifier, tsg_expr_t* expr) {
  tsg_type_t* cond_type = verify_expr(verifier, expr->ifelse.cond);
  if (cond_type && cond_type->kind != TSG_TYPE_BOOL) {
    error(verifier, &(expr->ifelse.cond->loc),
          "cond expr must have boolean type");
  }
  tsg_type_release(cond_type);

  tsg_type_t* thn_type = verify_block(verifier, expr->ifelse.thn);
  tsg_type_t* els_type = verify_block(verifier, expr->ifelse.els);

  if (thn_type == NULL || els_type == NULL) {
    return NULL;
  }

  if (thn_type->kind == TSG_TYPE_PEND && els_type->kind != TSG_TYPE_PEND) {
    tsg_type_release(thn_type);
    tsg_tyenv_set(verifier->func_env, expr->type_id, els_type);
    return els_type;
  } else if (thn_type->kind != TSG_TYPE_PEND &&
             els_type->kind == TSG_TYPE_PEND) {
    tsg_type_release(els_type);
    tsg_tyenv_set(verifier->func_env, expr->type_id, thn_type);
    return thn_type;
  } else if (tsg_type_equals(thn_type, els_type)) {
    tsg_type_release(els_type);
    tsg_tyenv_set(verifier->func_env, expr->type_id, thn_type);
    return thn_type;
  } else {
    error(verifier, &(expr->loc),
          "type miss match with thn_block and els_block");

    tsg_type_release(thn_type);
    tsg_type_release(els_type);
    return NULL;
  }
}

tsg_type_t* verify_expr_variable(tsg_verifier_t* verifier, tsg_expr_t* expr) {
  tsg_decl_t* decl = expr->variable.resolved;
  tsg_type_t* var_type = NULL;
  if (decl->depth == 0) {
    var_type = tsg_tyenv_get(verifier->root_env, decl->type_id);
  } else {
    var_type = tsg_tyenv_get(verifier->func_env, decl->type_id);
  }
  tsg_type_retain(var_type);
  tsg_tyenv_set(verifier->func_env, expr->type_id, var_type);
  return var_type;
}

tsg_type_t* verify_expr_number(tsg_verifier_t* verifier, tsg_expr_t* expr) {
  tsg_type_t* type = tsg_type_create();
  type->kind = TSG_TYPE_INT;
  tsg_tyenv_set(verifier->func_env, expr->type_id, type);
  return type;
}

tsg_type_arr_t* verify_expr_list(tsg_verifier_t* verifier,
                                 tsg_expr_list_t* list) {
  tsg_type_arr_t* arr = tsg_type_arr_create(list->size);
  tsg_type_t** p = arr->elem;

  tsg_expr_node_t* node = list->head;
  while (node) {
    *(p++) = verify_expr(verifier, node->expr);
    node = node->next;
  }

  return arr;
}
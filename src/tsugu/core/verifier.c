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
  tsg_errlist_t errors;
  tsg_tyenv_t* tyenv;
};

static void error(tsg_verifier_t* verifier, tsg_source_range_t* loc,
                  const char* format, ...);

static tsg_type_t* verify_poly(tsg_verifier_t* verifier, tsg_type_t* poly,
                               tsg_type_arr_t* args);
static void verify_func(tsg_verifier_t* verifier, tsg_func_t* func,
                        tsg_type_arr_t* arg_types);
static tsg_type_t* verify_block(tsg_verifier_t* verifier, tsg_block_t* block);
static void verify_func_list(tsg_verifier_t* verifier, tsg_func_list_t* list);
static tsg_type_t* verify_stmt_list(tsg_verifier_t* verifier,
                                    tsg_stmt_list_t* list);

static tsg_type_t* verify_stmt(tsg_verifier_t* verifier, tsg_stmt_t* stmt);
static tsg_type_t* verify_stmt_val(tsg_verifier_t* verifier, tsg_stmt_t* stmt);
static tsg_type_t* verify_stmt_expr(tsg_verifier_t* verifier, tsg_stmt_t* stmt);

static tsg_type_t* verify_expr(tsg_verifier_t* verifier, tsg_expr_t* expr);
static tsg_type_t* verify_expr_binary(tsg_verifier_t* verifier,
                                      tsg_expr_t* expr);
static tsg_type_t* verify_expr_call(tsg_verifier_t* verifier, tsg_expr_t* expr);
static tsg_type_t* verify_expr_ifelse(tsg_verifier_t* verifier,
                                      tsg_expr_t* expr);
static tsg_type_t* verify_expr_ident(tsg_verifier_t* verifier,
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

  tsg_errlist_init(&(verifier->errors));
  verifier->tyenv = NULL;

  return verifier;
}

void tsg_verifier_destroy(tsg_verifier_t* verifier) {
  tsg_errlist_release(&(verifier->errors));
  tsg_assert(verifier->tyenv == NULL);
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

bool tsg_verifier_verify(tsg_verifier_t* verifier, tsg_ast_t* ast) {
  tsg_func_t* root_func = ast->root;
  ast->tyenv = tsg_tyenv_create(root_func->tyset, NULL);
  tsg_type_arr_t* root_args = tsg_type_arr_create(0);

  tsg_assert(verifier->tyenv == NULL);
  verifier->tyenv = ast->tyenv;
  verify_func(verifier, root_func, root_args);
  verifier->tyenv = NULL;

  return verifier->errors.head == NULL;
}

tsg_type_t* verify_poly(tsg_verifier_t* verifier, tsg_type_t* poly,
                        tsg_type_arr_t* args) {
  tsg_assert(poly->kind == TSG_TYPE_POLY);
  tsg_assert(args->size == poly->poly.func->params->size);

  tsg_tyenv_t* tyenv = tsg_tymap_get(poly->poly.tymap, args);

  if (tyenv == NULL) {
    tyenv = tsg_tyenv_create(poly->poly.func->tyset, poly->poly.outer);
    tsg_tymap_add(poly->poly.tymap, args, tyenv);

    tsg_tyenv_t* stashed = verifier->tyenv;
    verifier->tyenv = tyenv;
    verify_func(verifier, poly->poly.func, args);
    verifier->tyenv = stashed;
  } else {
    tsg_type_arr_destroy(args);
  }

  return tsg_tyenv_get(tyenv, poly->poly.func->ftype);
}

void verify_func(tsg_verifier_t* verifier, tsg_func_t* func,
                 tsg_type_arr_t* arg_types) {
  tsg_assert(func->params->size == arg_types->size);

  tsg_type_t* func_type = tsg_type_create(TSG_TYPE_FUNC);
  func_type->func.params = arg_types;
  func_type->func.ret = tsg_type_create(TSG_TYPE_PEND);
  tsg_tyenv_set(verifier->tyenv, func->ftype, func_type);

  tsg_decl_node_t* decl = func->params->head;
  tsg_type_t** ptype = arg_types->elem;
  while (decl) {
    tsg_type_t* type = *ptype;
    tsg_tyenv_set(verifier->tyenv, decl->decl->object->tyvar, type);

    decl = decl->next;
    ptype++;
  }

  tsg_type_t* ret_type = verify_block(verifier, func->body);
  tsg_type_release(func_type->func.ret);
  func_type->func.ret = ret_type;
  tsg_type_release(func_type);
}

tsg_type_t* verify_block(tsg_verifier_t* verifier, tsg_block_t* block) {
  verify_func_list(verifier, block->funcs);
  return verify_stmt_list(verifier, block->stmts);
}

void verify_func_list(tsg_verifier_t* verifier, tsg_func_list_t* list) {
  tsg_func_node_t* node = list->head;
  while (node != NULL) {
    tsg_type_t* type = tsg_type_create(TSG_TYPE_POLY);
    type->poly.func = node->func;
    type->poly.outer = verifier->tyenv;
    type->poly.tymap = tsg_tymap_create();
    tsg_tyenv_set(verifier->tyenv, node->func->decl->object->tyvar, type);
    tsg_type_release(type);

    node = node->next;
  }
}

tsg_type_t* verify_stmt_list(tsg_verifier_t* verifier, tsg_stmt_list_t* list) {
  tsg_type_t* last_stmt_type = NULL;

  tsg_stmt_node_t* node = list->head;
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
  tsg_assert(stmt != NULL && stmt->kind == TSG_STMT_VAL);

  tsg_type_t* type = verify_expr(verifier, stmt->val.expr);
  tsg_tyenv_set(verifier->tyenv, stmt->val.decl->object->tyvar, type);

  return type;
}

tsg_type_t* verify_stmt_expr(tsg_verifier_t* verifier, tsg_stmt_t* stmt) {
  tsg_assert(stmt != NULL && stmt->kind == TSG_STMT_EXPR);
  return verify_expr(verifier, stmt->expr.expr);
}

tsg_type_t* verify_expr(tsg_verifier_t* verifier, tsg_expr_t* expr) {
  tsg_type_t* type = NULL;

  switch (expr->kind) {
    case TSG_EXPR_BINARY:
      type = verify_expr_binary(verifier, expr);
      break;

    case TSG_EXPR_CALL:
      type = verify_expr_call(verifier, expr);
      break;

    case TSG_EXPR_IFELSE:
      type = verify_expr_ifelse(verifier, expr);
      break;

    case TSG_EXPR_IDENT:
      type = verify_expr_ident(verifier, expr);
      break;

    case TSG_EXPR_NUMBER:
      type = verify_expr_number(verifier, expr);
      break;
  }

  if (type != NULL) {
    tsg_tyenv_set(verifier->tyenv, expr->tyvar, type);
  }

  return type;
}

tsg_type_t* verify_expr_binary(tsg_verifier_t* verifier, tsg_expr_t* expr) {
  tsg_assert(expr != NULL && expr->kind == TSG_EXPR_BINARY);

  tsg_type_t* lhs_type = verify_expr(verifier, expr->binary.lhs);
  tsg_type_t* rhs_type = verify_expr(verifier, expr->binary.rhs);

  if (lhs_type == NULL || rhs_type == NULL) {
    return NULL;
  }

  tsg_type_t* ret_type = tsg_type_binary(expr->binary.op, lhs_type, rhs_type);
  if (ret_type == NULL) {
    error(verifier, &(expr->loc), "incompatible type");
  }

  tsg_type_release(lhs_type);
  tsg_type_release(rhs_type);

  return ret_type;
}

tsg_type_t* verify_expr_call(tsg_verifier_t* verifier, tsg_expr_t* expr) {
  tsg_assert(expr != NULL && expr->kind == TSG_EXPR_CALL);

  tsg_type_t* callee_type = verify_expr(verifier, expr->call.callee);
  tsg_type_arr_t* arg_types = verify_expr_list(verifier, expr->call.args);

  if (callee_type == NULL) {
    return NULL;
  }

  if (callee_type->kind != TSG_TYPE_POLY) {
    error(verifier, &(expr->call.callee->loc), "callee is not a function");
    return NULL;
  }

  if (expr->call.args->size < callee_type->poly.func->params->size) {
    error(verifier, &(expr->loc), "too few arguments");
    return NULL;
  }
  if (expr->call.args->size > callee_type->poly.func->params->size) {
    error(verifier, &(expr->loc), "too many arguments");
    return NULL;
  }

  tsg_type_t* func_type = verify_poly(verifier, callee_type, arg_types);
  tsg_assert(func_type->kind == TSG_TYPE_FUNC);
  tsg_type_release(callee_type);

  tsg_type_t* type = func_type->func.ret;
  if (type != NULL) {
    tsg_type_retain(type);
  }

  return type;
}

tsg_type_t* verify_expr_ifelse(tsg_verifier_t* verifier, tsg_expr_t* expr) {
  tsg_assert(expr != NULL && expr->kind == TSG_EXPR_IFELSE);

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

  tsg_type_t* ret_type = tsg_type_unify(thn_type, els_type);
  if (ret_type == NULL) {
    error(verifier, &(expr->loc),
          "type miss match with thn_block and els_block");
  }

  tsg_type_release(thn_type);
  tsg_type_release(els_type);

  return ret_type;
}

tsg_type_t* verify_expr_ident(tsg_verifier_t* verifier, tsg_expr_t* expr) {
  tsg_assert(expr != NULL && expr->kind == TSG_EXPR_IDENT);
  tsg_assert(expr->ident.object != NULL);

  tsg_type_t* type = tsg_tyenv_get(verifier->tyenv, expr->ident.object->tyvar);
  tsg_type_retain(type);
  return type;
}

tsg_type_t* verify_expr_number(tsg_verifier_t* verifier, tsg_expr_t* expr) {
  (void)verifier;
  tsg_assert(expr != NULL && expr->kind == TSG_EXPR_NUMBER);

  return tsg_type_create(TSG_TYPE_INT);
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

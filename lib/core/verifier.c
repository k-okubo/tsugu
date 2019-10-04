
#include <tsugu/core/verifier.h>

#include <tsugu/core/memory.h>
#include <string.h>

struct tsg_verifier_s {
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
static tsg_type_t* verify_expr_variable(tsg_expr_t* expr);
static tsg_type_t* verify_expr_number(void);

static tsg_type_arr_t* verify_expr_list(tsg_verifier_t* verifier,
                                        tsg_expr_list_t* list);

tsg_verifier_t* tsg_verifier_create(void) {
  tsg_verifier_t* verifier = tsg_malloc_obj(tsg_verifier_t);
  if (verifier == NULL) {
    return NULL;
  }

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

bool tsg_verifier_verify(tsg_verifier_t* verifier, tsg_ast_t* ast) {
  verify_ast(verifier, ast);
  return verifier->errors.head == NULL;
}

void verify_ast(tsg_verifier_t* verifier, tsg_ast_t* ast) {
  tsg_func_t* main_func = NULL;

  tsg_func_node_t* node = ast->functions->head;
  while (node) {
    tsg_type_t* type = tsg_type_create();
    type->kind = TSG_TYPE_FUNC;
    type->func.params = NULL;
    type->func.ret = NULL;
    type->func.func = node->func;
    node->func->decl->type = type;

    if (memcmp(node->func->decl->name->buffer, "main", 4) == 0) {
      main_func = node->func;
    }

    node = node->next;
  }

  tsg_type_arr_t* main_arg = tsg_type_arr_create(0);
  verify_func(verifier, main_func, main_arg);
}

void verify_func(tsg_verifier_t* verifier, tsg_func_t* func,
                 tsg_type_arr_t* arg_types) {
  if (func->args->size != arg_types->size) {
    return;
  }

  tsg_type_t* func_type = func->decl->type;
  func_type->kind = TSG_TYPE_FUNC;
  func_type->func.params = arg_types;
  func_type->func.ret = tsg_type_create();
  func_type->func.ret->kind = TSG_TYPE_PEND;

  tsg_decl_node_t* decl = func->args->head;
  tsg_type_t** ptype = arg_types->elem;
  while (decl) {
    tsg_type_t* type = *ptype;
    tsg_type_retain(type);

    decl->decl->type = type;

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
}

tsg_type_t* verify_stmt_val(tsg_verifier_t* verifier, tsg_stmt_t* stmt) {
  tsg_type_t* type = verify_expr(verifier, stmt->val.expr);
  stmt->val.decl->type = type;

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
      return verify_expr_variable(expr);

    case TSG_EXPR_NUMBER:
      return verify_expr_number();
  }
}

tsg_type_t* verify_expr_binary(tsg_verifier_t* verifier, tsg_expr_t* expr) {
  tsg_type_t* lhs_type = verify_expr(verifier, expr->binary.lhs);
  tsg_type_t* rhs_type = verify_expr(verifier, expr->binary.rhs);

  if (lhs_type == NULL || rhs_type == NULL) {
    return NULL;
  }

  tsg_type_t* type = tsg_type_create();

  if (lhs_type->kind == TSG_TYPE_INT && rhs_type->kind == TSG_TYPE_INT) {
    type->kind = TSG_TYPE_INT;
  } else if (lhs_type->kind == TSG_TYPE_INT &&
             rhs_type->kind == TSG_TYPE_PEND) {
    type->kind = TSG_TYPE_INT;
  } else if (lhs_type->kind == TSG_TYPE_PEND &&
             rhs_type->kind == TSG_TYPE_INT) {
    type->kind = TSG_TYPE_INT;
  } else if (lhs_type->kind == TSG_TYPE_PEND &&
             rhs_type->kind == TSG_TYPE_PEND) {
    type->kind = TSG_TYPE_PEND;
  } else {
    error(verifier, &(expr->loc), "incompatible type");
    tsg_type_release(type);
    type = NULL;
  }

  tsg_type_release(lhs_type);
  tsg_type_release(rhs_type);

  return type;
}

tsg_type_t* verify_expr_call(tsg_verifier_t* verifier, tsg_expr_t* expr) {
  tsg_type_t* callee_type = verify_expr(verifier, expr->call.callee);
  tsg_type_arr_t* arg_types = verify_expr_list(verifier, expr->call.args);

  if (callee_type == NULL) {
    return NULL;
  }

  if (callee_type->kind != TSG_TYPE_FUNC) {
    error(verifier, &(expr->call.callee->loc), "callee is not a function");
    return NULL;
  }

  if (callee_type->func.ret != NULL) {
    // already typed
    if (expr->call.args->size < callee_type->func.params->size) {
      error(verifier, &(expr->loc), "too few arguments");
      return NULL;
    }
    if (expr->call.args->size > callee_type->func.params->size) {
      error(verifier, &(expr->loc), "too many arguments");
      return NULL;
    }

    tsg_type_t** param_type = callee_type->func.params->elem;
    tsg_type_t** arg_type = arg_types->elem;

    while (arg_type < arg_types->elem + arg_types->size) {
      if ((*param_type)->kind == TSG_TYPE_FUNC) {
        if ((*arg_type)->kind == TSG_TYPE_FUNC &&
            (*arg_type)->func.ret == NULL) {
          verify_func(verifier, (*arg_type)->func.func,
                      tsg_type_arr_dup((*param_type)->func.params));
        }
      }

      if (tsg_type_equals(*param_type, *arg_type) == false) {
        error(verifier, &(expr->loc), "arg type miss match");
      }

      param_type++;
      arg_type++;
    }

    tsg_type_release(callee_type);
    tsg_type_arr_destroy(arg_types);
  } else {
    tsg_func_t* func = callee_type->func.func;

    if (expr->call.args->size < func->args->size) {
      error(verifier, &(expr->loc), "too few arguments");
      return NULL;
    }
    if (expr->call.args->size > func->args->size) {
      error(verifier, &(expr->loc), "too many arguments");
      return NULL;
    }

    verify_func(verifier, func, arg_types);
  }

  tsg_type_t* type = callee_type->func.ret;
  tsg_type_retain(type);
  return type;
}

tsg_type_t* verify_expr_ifelse(tsg_verifier_t* verifier, tsg_expr_t* expr) {
  tsg_type_t* cond_type = verify_expr(verifier, expr->ifelse.cond);
  if (cond_type && cond_type->kind != TSG_TYPE_INT) {
    error(verifier, &(expr->ifelse.cond->loc),
          "cond expr must have integer type");
  }
  tsg_type_release(cond_type);

  tsg_type_t* thn_type = verify_block(verifier, expr->ifelse.thn);
  tsg_type_t* els_type = verify_block(verifier, expr->ifelse.els);

  if (thn_type == NULL || els_type == NULL) {
    return NULL;
  }

  if (thn_type->kind == TSG_TYPE_PEND && els_type->kind != TSG_TYPE_PEND) {
    tsg_type_release(thn_type);
    return els_type;
  } else if (thn_type->kind != TSG_TYPE_PEND &&
             els_type->kind == TSG_TYPE_PEND) {
    tsg_type_release(els_type);
    return thn_type;
  } else if (tsg_type_equals(thn_type, els_type)) {
    tsg_type_release(els_type);
    return thn_type;
  } else {
    error(verifier, &(expr->loc),
          "type miss match with thn_block and els_block");

    tsg_type_release(thn_type);
    tsg_type_release(els_type);
    return NULL;
  }
}

tsg_type_t* verify_expr_variable(tsg_expr_t* expr) {
  tsg_decl_t* decl = expr->variable.resolved;
  tsg_type_t* var_type = decl->type;
  tsg_type_retain(var_type);
  return var_type;
}

tsg_type_t* verify_expr_number(void) {
  tsg_type_t* type = tsg_type_create();
  type->kind = TSG_TYPE_INT;
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

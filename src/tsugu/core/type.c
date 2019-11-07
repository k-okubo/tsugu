/*--------------------------------------- vi: set ft=c ts=2 sw=2 et: --*-c-*--*/
/**
 * @file type.c
 *
 ** --------------------------------------------------------------------------*/

#include <tsugu/core/type.h>

#include <tsugu/core/memory.h>
#include <tsugu/core/platform.h>
#include <tsugu/core/tymap.h>

static void destroy_type(tsg_type_t* type);
static void destroy_type_func(tsg_type_t* type);
static void destroy_type_poly(tsg_type_t* type);

static tsg_type_t* type_op_eq(tsg_type_t* lhs, tsg_type_t* rhs);
static tsg_type_t* type_op_cmp(tsg_type_t* lhs, tsg_type_t* rhs);
static tsg_type_t* type_op_arith(tsg_type_t* lhs, tsg_type_t* rhs);

tsg_type_t* tsg_type_create(tsg_type_kind_t kind) {
  tsg_type_t* type = tsg_malloc_obj(tsg_type_t);

  type->kind = kind;
  type->nrefs = 1;

  return type;
}

void tsg_type_retain(tsg_type_t* type) {
  tsg_assert(type != NULL);
  tsg_assert(type->nrefs > 0);

  type->nrefs += 1;
}

void tsg_type_release(tsg_type_t* type) {
  tsg_assert(type != NULL);
  tsg_assert(type->nrefs > 0);

  type->nrefs -= 1;

  if (type->nrefs <= 0) {
    destroy_type(type);
  }
}

void destroy_type(tsg_type_t* type) {
  switch (type->kind) {
    case TSG_TYPE_BOOL:
      break;

    case TSG_TYPE_INT:
      break;

    case TSG_TYPE_FUNC:
      destroy_type_func(type);
      break;

    case TSG_TYPE_POLY:
      destroy_type_poly(type);
      break;

    case TSG_TYPE_PEND:
      break;
  }

  tsg_free(type);
}

void destroy_type_func(tsg_type_t* type) {
  tsg_assert(type->kind == TSG_TYPE_FUNC);
  tsg_type_arr_destroy(type->func.params);
  tsg_type_release(type->func.ret);
}

void destroy_type_poly(tsg_type_t* type) {
  tsg_assert(type->kind == TSG_TYPE_POLY);
  // `type->poly.outer` is a reference
  tsg_tymap_destroy(type->poly.tymap);
}

bool tsg_type_equals(tsg_type_t* a, tsg_type_t* b) {
  tsg_assert(a != NULL);
  tsg_assert(b != NULL);

  if (a->kind == b->kind) {
    switch (a->kind) {
      case TSG_TYPE_BOOL:
        return true;

      case TSG_TYPE_INT:
        return true;

      case TSG_TYPE_FUNC:
        if (tsg_type_equals(a->func.ret, b->func.ret) == false) {
          return false;
        }
        if (tsg_type_arr_equals(a->func.params, b->func.params) == false) {
          return false;
        }
        return true;

      case TSG_TYPE_POLY:
        return a->poly.func == b->poly.func;

      case TSG_TYPE_PEND:
        return a == b;
    }
  } else {
    return false;
  }
}

tsg_type_t* tsg_type_unify(tsg_type_t* a, tsg_type_t* b) {
  tsg_assert(a != NULL);
  tsg_assert(b != NULL);

  if (tsg_type_equals(a, b)) {
    tsg_type_retain(a);
    return a;
  } else if (a->kind == TSG_TYPE_PEND && b->kind != TSG_TYPE_PEND) {
    tsg_type_retain(b);
    return b;
  } else if (a->kind != TSG_TYPE_PEND && b->kind == TSG_TYPE_PEND) {
    tsg_type_retain(a);
    return a;
  } else {
    return NULL;
  }
}

tsg_type_t* tsg_type_binary(tsg_token_kind_t op, tsg_type_t* lhs,
                            tsg_type_t* rhs) {
  tsg_assert(lhs != NULL);
  tsg_assert(rhs != NULL);

  switch (op) {
    case TSG_TOKEN_EQ:
      return type_op_eq(lhs, rhs);

    case TSG_TOKEN_LT:
    case TSG_TOKEN_GT:
      return type_op_cmp(lhs, rhs);

    case TSG_TOKEN_ADD:
    case TSG_TOKEN_SUB:
    case TSG_TOKEN_MUL:
    case TSG_TOKEN_DIV:
      return type_op_arith(lhs, rhs);

    default:
      tsg_assert(false);
      return NULL;
  }
}

tsg_type_t* type_op_eq(tsg_type_t* lhs, tsg_type_t* rhs) {
  if (tsg_type_equals(lhs, rhs)) {
    return tsg_type_create(TSG_TYPE_BOOL);
  } else {
    return NULL;
  }
}

tsg_type_t* type_op_cmp(tsg_type_t* lhs, tsg_type_t* rhs) {
  if (lhs->kind == TSG_TYPE_INT) {
    if (rhs->kind == TSG_TYPE_INT) {
      return tsg_type_create(TSG_TYPE_BOOL);
    } else if (rhs->kind == TSG_TYPE_PEND) {
      return tsg_type_create(TSG_TYPE_BOOL);
    } else {
      return NULL;
    }
  } else if (lhs->kind == TSG_TYPE_PEND) {
    if (rhs->kind == TSG_TYPE_INT) {
      return tsg_type_create(TSG_TYPE_BOOL);
    } else if (rhs->kind == TSG_TYPE_PEND) {
      return tsg_type_create(TSG_TYPE_BOOL);
    } else {
      return NULL;
    }
  } else {
    return NULL;
  }
}

tsg_type_t* type_op_arith(tsg_type_t* lhs, tsg_type_t* rhs) {
  if (lhs->kind == TSG_TYPE_INT) {
    if (rhs->kind == TSG_TYPE_INT) {
      tsg_type_retain(lhs);
      return lhs;
    } else if (rhs->kind == TSG_TYPE_PEND) {
      tsg_type_retain(lhs);
      return lhs;
    } else {
      return NULL;
    }
  } else if (lhs->kind == TSG_TYPE_PEND) {
    if (rhs->kind == TSG_TYPE_INT) {
      tsg_type_retain(rhs);
      return rhs;
    } else if (rhs->kind == TSG_TYPE_PEND) {
      return tsg_type_create(TSG_TYPE_INT);
    } else {
      return NULL;
    }
  } else {
    return NULL;
  }
}

tsg_type_arr_t* tsg_type_arr_create(size_t size) {
  tsg_type_arr_t* arr = tsg_malloc_obj(tsg_type_arr_t);

  if (size > 0) {
    arr->elem = tsg_malloc_arr(tsg_type_t*, size);
    tsg_memset(arr->elem, 0, sizeof(tsg_type_t*) * size);
    arr->size = size;
  } else {
    arr->elem = NULL;
    arr->size = 0;
  }

  return arr;
}

tsg_type_arr_t* tsg_type_arr_dup(const tsg_type_arr_t* src) {
  tsg_assert(src != NULL);
  tsg_type_arr_t* dst = tsg_malloc_obj(tsg_type_arr_t);

  if (src->size > 0) {
    dst->elem = tsg_malloc_arr(tsg_type_t*, src->size);
    dst->size = src->size;

    tsg_type_t** p = src->elem;
    tsg_type_t** end = p + src->size;
    tsg_type_t** q = dst->elem;
    while (p < end) {
      tsg_type_t* type = *(p++);
      tsg_type_retain(type);
      *(q++) = type;
    }
  } else {
    dst->elem = NULL;
    dst->size = 0;
  }

  return dst;
}

void tsg_type_arr_destroy(tsg_type_arr_t* arr) {
  if (arr == NULL) {
    return;
  }

  tsg_type_t** p = arr->elem;
  tsg_type_t** end = p + arr->size;
  while (p < end) {
    tsg_type_release(*p);
    p++;
  }

  tsg_free(arr->elem);
  tsg_free(arr);
}

bool tsg_type_arr_equals(tsg_type_arr_t* a, tsg_type_arr_t* b) {
  tsg_assert(a != NULL);
  tsg_assert(b != NULL);

  if (a->size != b->size) {
    return false;
  }

  tsg_type_t** p = a->elem;
  tsg_type_t** end = p + a->size;
  tsg_type_t** q = b->elem;

  while (p < end) {
    if (tsg_type_equals(*p, *q) == false) {
      return false;
    }
    p++;
    q++;
  }

  return true;
}

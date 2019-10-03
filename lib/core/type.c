
#include <tsugu/core/type.h>

#include <tsugu/core/memory.h>
#include <string.h>

static void destroy_type(tsg_type_t* type);
static void destory_type_func(tsg_type_t* type);

tsg_type_t* tsg_type_create(void) {
  tsg_type_t* type = tsg_malloc_obj(tsg_type_t);
  if (type == NULL) {
    return NULL;
  }

  type->nrefs = 1;
  return type;
}

void tsg_type_retain(tsg_type_t* type) {
  if (type == NULL) {
    return;
  }

  type->nrefs += 1;
}

void tsg_type_release(tsg_type_t* type) {
  if (type == NULL) {
    return;
  }

  type->nrefs -= 1;

  if (type->nrefs <= 0) {
    destroy_type(type);
  }
}

void destroy_type(tsg_type_t* type) {
  if (type == NULL) {
    return;
  }

  switch (type->kind) {
    case TSG_TYPE_INT:
      break;

    case TSG_TYPE_FUNC:
      destory_type_func(type);
      break;
  }

  tsg_free(type);
}

void destory_type_func(tsg_type_t* type) {
  tsg_type_release(type->func.ret);
  tsg_type_arr_destroy(type->func.params);
}

bool tsg_type_equals(tsg_type_t* a, tsg_type_t* b) {
  if (a->kind != b->kind) {
    return false;
  }

  switch (a->kind) {
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
  }
}

tsg_type_arr_t* tsg_type_arr_create(size_t size) {
  tsg_type_arr_t* arr = tsg_malloc_obj(tsg_type_arr_t);
  if (arr == NULL) {
    return NULL;
  }

  if (size > 0) {
    arr->elem = tsg_malloc_arr(tsg_type_t*, size);
    memset(arr->elem, 0, sizeof(tsg_type_t*) * size);
    arr->size = size;
  } else {
    arr->elem = NULL;
    arr->size = 0;
  }

  return arr;
}

tsg_type_arr_t* tsg_type_arr_dup(const tsg_type_arr_t* arr) {
  tsg_type_arr_t* cpy = tsg_malloc_obj(tsg_type_arr_t);
  if (cpy == NULL) {
    return NULL;
  }

  if (arr->size > 0) {
    cpy->elem = tsg_malloc_arr(tsg_type_t*, arr->size);
    cpy->size = arr->size;

    tsg_type_t** p = arr->elem;
    tsg_type_t** end = p + arr->size;
    tsg_type_t** out = cpy->elem;
    while (p < end) {
      tsg_type_t* type = *(p++);
      tsg_type_retain(type);
      *(out++) = type;
    }
  } else {
    cpy->elem = NULL;
    cpy->size = 0;
  }

  return cpy;
}

void tsg_type_arr_destroy(tsg_type_arr_t* arr) {
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

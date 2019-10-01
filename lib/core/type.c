
#include <tsugu/core/type.h>

#include <tsugu/core/memory.h>

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
}

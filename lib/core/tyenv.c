
#include <tsugu/core/tyenv.h>

#include <tsugu/core/memory.h>

struct tsg_tyenv_s {
  tsg_tyenv_t* outer;
  tsg_type_t** arr;
  int32_t size;
};

tsg_tyenv_t* tsg_tyenv_create(tsg_tyenv_t* outer, int32_t size) {
  tsg_tyenv_t* tyenv = tsg_malloc_obj(tsg_tyenv_t);
  if (tyenv == NULL) {
    return NULL;
  }

  tyenv->outer = outer;
  tyenv->arr = tsg_malloc_arr(tsg_type_t*, size);
  tyenv->size = size;

  return tyenv;
}

void tsg_tyenv_destroy(tsg_tyenv_t* tyenv) {
  tsg_free(tyenv->arr);
  tsg_free(tyenv);
}

void tsg_tyenv_set(tsg_tyenv_t* tyenv, int32_t id, tsg_type_t* type) {
  tyenv->arr[id] = type;
}

tsg_type_t* tsg_tyenv_get(tsg_tyenv_t* tyenv, int32_t id) {
  return tyenv->arr[id];
}

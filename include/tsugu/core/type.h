
#ifndef TSUGU_CORE_TYPE_H
#define TSUGU_CORE_TYPE_H

#include <stddef.h>
#include <stdint.h>

typedef enum {
  TSG_TYPE_INT,
  TSG_TYPE_FUNC,
} tsg_type_kind_t;

typedef struct tsg_func_s tsg_func_t;

typedef struct tsg_type_s tsg_type_t;
typedef struct tsg_type_arr_s tsg_type_arr_t;

struct tsg_type_func_s {
  tsg_type_arr_t* params;
  tsg_type_t* ret;
  tsg_func_t* func;
};

struct tsg_type_s {
  tsg_type_kind_t kind;

  union {
    struct tsg_type_func_s func;
  };

  int32_t nrefs;
};

struct tsg_type_arr_s {
  tsg_type_t** elem;
  size_t size;
};

tsg_type_t* tsg_type_create(void);
void tsg_type_retain(tsg_type_t* type);
void tsg_type_release(tsg_type_t* type);

tsg_type_arr_t* tsg_type_arr_create(size_t size);
void tsg_type_arr_destroy(tsg_type_arr_t* arr);

#endif

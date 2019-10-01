
#ifndef TSUGU_CORE_TYPE_H
#define TSUGU_CORE_TYPE_H

#include <stddef.h>
#include <stdint.h>

typedef enum {
  TSG_TYPE_INT,
  TSG_TYPE_FUNC,
} tsg_type_kind_t;

typedef struct tsg_type_s tsg_type_t;

struct tsg_type_func_s {
  tsg_type_t* ret;
  size_t n_args;
};

struct tsg_type_s {
  tsg_type_kind_t kind;

  union {
    struct tsg_type_func_s func;
  };

  int32_t nrefs;
};

tsg_type_t* tsg_type_create(void);
void tsg_type_retain(tsg_type_t* type);
void tsg_type_release(tsg_type_t* type);

#endif

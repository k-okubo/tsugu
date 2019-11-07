/*--------------------------------------- vi: set ft=c ts=2 sw=2 et: --*-c-*--*/
/**
 * @file type.h
 *
 ** --------------------------------------------------------------------------*/

#ifndef TSUGU_CORE_TYPE_H
#define TSUGU_CORE_TYPE_H

#include <tsugu/core/token.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
  TSG_TYPE_BOOL,
  TSG_TYPE_INT,
  TSG_TYPE_FUNC,
  TSG_TYPE_POLY,
  TSG_TYPE_PEND,
} tsg_type_kind_t;

typedef struct tsg_func_s tsg_func_t;
typedef struct tsg_tyenv_s tsg_tyenv_t;
typedef struct tsg_tymap_s tsg_tymap_t;

typedef struct tsg_type_s tsg_type_t;
typedef struct tsg_type_arr_s tsg_type_arr_t;

struct tsg_type_func_s {
  tsg_type_arr_t* params;
  tsg_type_t* ret;
};

struct tsg_type_poly_s {
  tsg_func_t* func;
  tsg_tyenv_t* outer;
  tsg_tymap_t* tymap;
};

struct tsg_type_s {
  tsg_type_kind_t kind;

  union {
    struct tsg_type_func_s func;
    struct tsg_type_poly_s poly;
  };

  int32_t nrefs;
};

struct tsg_type_arr_s {
  tsg_type_t** elem;
  size_t size;
};

tsg_type_t* tsg_type_create(tsg_type_kind_t kind);
void tsg_type_retain(tsg_type_t* type);
void tsg_type_release(tsg_type_t* type);
bool tsg_type_equals(tsg_type_t* a, tsg_type_t* b);

tsg_type_t* tsg_type_unify(tsg_type_t* a, tsg_type_t* b);
tsg_type_t* tsg_type_binary(tsg_token_kind_t op, tsg_type_t* lhs,
                            tsg_type_t* rhs);

tsg_type_arr_t* tsg_type_arr_create(size_t size);
tsg_type_arr_t* tsg_type_arr_dup(const tsg_type_arr_t* src);
void tsg_type_arr_destroy(tsg_type_arr_t* arr);
bool tsg_type_arr_equals(tsg_type_arr_t* a, tsg_type_arr_t* b);

#ifdef __cplusplus
}
#endif

#endif

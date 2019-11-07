/*--------------------------------------- vi: set ft=c ts=2 sw=2 et: --*-c-*--*/
/**
 * @file tyenv.h
 *
 ** --------------------------------------------------------------------------*/

#ifndef TSUGU_CORE_TYENV_H
#define TSUGU_CORE_TYENV_H

#include <tsugu/core/type.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct tsg_tyset_s tsg_tyset_t;
typedef struct tsg_tyvar_s tsg_tyvar_t;
typedef struct tsg_tyenv_s tsg_tyenv_t;

struct tsg_tyset_s {
  tsg_tyset_t* outer;
  int32_t depth;
  int32_t n_entries;
};

struct tsg_tyvar_s {
  tsg_tyset_t* tyset;
  int32_t index;
};

struct tsg_tyenv_s {
  tsg_tyenv_t* outer;
  tsg_tyset_t* tyset;
  tsg_type_t** arr;
  int32_t size;
};

tsg_tyset_t* tsg_tyset_create(tsg_tyset_t* outer);
void tsg_tyset_destroy(tsg_tyset_t* tyset);

tsg_tyvar_t* tsg_tyvar_create(tsg_tyset_t* tyset);
void tsg_tyvar_destroy(tsg_tyvar_t* tyvar);

tsg_tyenv_t* tsg_tyenv_create(tsg_tyset_t* tyset, tsg_tyenv_t* outer);
void tsg_tyenv_destroy(tsg_tyenv_t* tyenv);

void tsg_tyenv_set(tsg_tyenv_t* tyenv, tsg_tyvar_t* tyvar, tsg_type_t* type);
tsg_type_t* tsg_tyenv_get(tsg_tyenv_t* tyenv, tsg_tyvar_t* tyvar);

#ifdef __cplusplus
}
#endif

#endif

/*--------------------------------------- vi: set ft=c ts=2 sw=2 et: --*-c-*--*/
/**
 * @file tymap.h
 *
 ** --------------------------------------------------------------------------*/

#ifndef TSUGU_CORE_TYMAP_H
#define TSUGU_CORE_TYMAP_H

#include <tsugu/core/tyenv.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct tsg_tymap_s tsg_tymap_t;

tsg_tymap_t* tsg_tymap_create(void);
void tsg_tymap_destroy(tsg_tymap_t* tymap);

void tsg_tymap_add(tsg_tymap_t* tymap, tsg_type_arr_t* key, tsg_tyenv_t* env);
tsg_tyenv_t* tsg_tymap_get(tsg_tymap_t* tymap, tsg_type_arr_t* key);

#ifdef __cplusplus
}
#endif

#endif

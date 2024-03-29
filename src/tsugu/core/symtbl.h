/*--------------------------------------- vi: set ft=c ts=2 sw=2 et: --*-c-*--*/
/**
 * @file symtbl.h
 *
 ** --------------------------------------------------------------------------*/

#ifndef TSUGU_CORE_SYMTBL_H
#define TSUGU_CORE_SYMTBL_H

#include <tsugu/core/scope.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct tsg_symtbl_s tsg_symtbl_t;

tsg_symtbl_t* tsg_symtbl_create(void);
void tsg_symtbl_destroy(tsg_symtbl_t* symtbl);
void tsg_symtbl_clear(tsg_symtbl_t* symtbl);

bool tsg_symtbl_insert(tsg_symtbl_t* symtbl, tsg_ident_t* ident,
                       tsg_member_t* member);
tsg_member_t* tsg_symtbl_lookup(tsg_symtbl_t* symtbl, tsg_ident_t* ident);

#ifdef __cplusplus
}
#endif

#endif

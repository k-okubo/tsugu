/*--------------------------------------- vi: set ft=c ts=2 sw=2 et: --*-c-*--*/
/**
 * @file scope.h
 *
 ** --------------------------------------------------------------------------*/

#ifndef TSUGU_CORE_SCOPE_H
#define TSUGU_CORE_SCOPE_H

#include <tsugu/core/ast.h>
#include <tsugu/core/frame.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct tsg_scope_s tsg_scope_t;

tsg_scope_t* tsg_scope_create(tsg_scope_t* outer);
void tsg_scope_destroy(tsg_scope_t* scope);

bool tsg_scope_add(tsg_scope_t* scope, tsg_ident_t* ident,
                   tsg_member_t* member);
tsg_member_t* tsg_scope_find(tsg_scope_t* scope, tsg_ident_t* ident);

#ifdef __cplusplus
}
#endif

#endif

/*--------------------------------------- vi: set ft=c ts=2 sw=2 et: --*-c-*--*/
/**
 * @file verifier.h
 *
 ** --------------------------------------------------------------------------*/

#ifndef TSUGU_CORE_VERIFIER_H
#define TSUGU_CORE_VERIFIER_H

#include <tsugu/core/ast.h>
#include <tsugu/core/error.h>
#include <tsugu/core/tyenv.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct tsg_verifier_s tsg_verifier_t;

tsg_verifier_t* tsg_verifier_create(void);
void tsg_verifier_destroy(tsg_verifier_t* verifier);

bool tsg_verifier_verify(tsg_verifier_t* verifier, tsg_ast_t* ast,
                         tsg_tyenv_t** root_env);
void tsg_verifier_error(const tsg_verifier_t* verifier, tsg_errlist_t* errors);

#ifdef __cplusplus
}
#endif

#endif


#ifndef TSUGU_CORE_RESOLVER_H
#define TSUGU_CORE_RESOLVER_H

#include <tsugu/core/ast.h>
#include <tsugu/core/error.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct tsg_resolver_s tsg_resolver_t;

tsg_resolver_t* tsg_resolver_create(void);
void tsg_resolver_destroy(tsg_resolver_t* resolver);

bool tsg_resolver_resolve(tsg_resolver_t* resolver, tsg_ast_t* ast);
void tsg_resolver_error(const tsg_resolver_t* resolver, tsg_errlist_t* errors);

#ifdef __cplusplus
}
#endif

#endif

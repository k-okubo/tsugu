
#ifndef TSUGU_CORE_RESOLVER_H
#define TSUGU_CORE_RESOLVER_H

#include <tsugu/core/ast.h>
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct tsg_resolver_s tsg_resolver_t;

tsg_resolver_t* tsg_resolver_create(void);
void tsg_resolver_destroy(tsg_resolver_t* resolver);
void tsg_resolver_clear(tsg_resolver_t* resolver);

bool tsg_resolver_insert(tsg_resolver_t* resolver, tsg_decl_t* decl);
bool tsg_resolver_lookup(tsg_resolver_t* resolver, tsg_ident_t* key,
                         tsg_decl_t** outval);

#ifdef __cplusplus
}
#endif

#endif


#ifndef TSUGU_CORE_TYENV_H
#define TSUGU_CORE_TYENV_H

#include <tsugu/core/type.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct tsg_tyenv_s tsg_tyenv_t;

tsg_tyenv_t* tsg_tyenv_create(tsg_tyenv_t* outer, int32_t size);
void tsg_tyenv_destroy(tsg_tyenv_t* tyenv);

void tsg_tyenv_set(tsg_tyenv_t* tyenv, int32_t id, tsg_type_t* type);
tsg_type_t* tsg_tyenv_get(tsg_tyenv_t* tyenv, int32_t id);

#ifdef __cplusplus
}
#endif

#endif

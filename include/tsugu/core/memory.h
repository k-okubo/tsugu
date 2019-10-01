
#ifndef TSUGU_CORE_MEMORY_H
#define TSUGU_CORE_MEMORY_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

void* tsg_malloc(size_t size);
void tsg_free(void* ptr);

#define tsg_malloc_obj(T) ((T*)tsg_malloc(sizeof(T)))
#define tsg_malloc_arr(T, n) ((T*)tsg_malloc(sizeof(T) * (n)))

#ifdef __cplusplus
}
#endif

#endif

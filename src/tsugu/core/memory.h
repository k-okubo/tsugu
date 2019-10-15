/*--------------------------------------- vi: set ft=c ts=2 sw=2 et: --*-c-*--*/
/**
 * @file memory.h
 *
 ** --------------------------------------------------------------------------*/

#ifndef TSUGU_CORE_MEMORY_H
#define TSUGU_CORE_MEMORY_H

#include <tsugu/core/platform.h>

#ifdef __cplusplus
extern "C" {
#endif

#define tsg_malloc_obj(T) ((T*)tsg_malloc(sizeof(T)))
#define tsg_malloc_arr(T, n) ((T*)tsg_malloc(sizeof(T) * (n)))

#ifdef __cplusplus
}
#endif

#endif

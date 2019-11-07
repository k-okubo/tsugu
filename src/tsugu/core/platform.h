/*--------------------------------------- vi: set ft=c ts=2 sw=2 et: --*-c-*--*/
/**
 * @file platform.h
 *
 ** --------------------------------------------------------------------------*/

#ifndef TSUGU_CORE_PLATFORM_H
#define TSUGU_CORE_PLATFORM_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

void* tsg_malloc(size_t size);
void tsg_free(void* ptr);

int tsg_memcmp(const void* lhs, const void* rhs, size_t count);
void* tsg_memcpy(void* dst, const void* src, size_t count);
void* tsg_memset(void* dst, int ch, size_t count);
size_t tsg_strlen(const char* str);

#ifdef NDEBUG
#define tsg_assert(expr) ((void)(0))
#else
void tsg_assert_failure(const char* expr, const char* file, int line,
                        const char* func);

#define tsg_assert(expr) \
  ((expr) ? (void)(0) : tsg_assert_failure(#expr, __FILE__, __LINE__, __func__))
#endif

#ifdef __cplusplus
}
#endif

#endif

/*--------------------------------------- vi: set ft=c ts=2 sw=2 et: --*-c-*--*/
/**
 * @file linux.c
 *
 ** --------------------------------------------------------------------------*/

#include <tsugu/core/platform.h>
#include <stdlib.h>
#include <string.h>

void* tsg_malloc(size_t size) {
  return malloc(size);
}

void tsg_free(void* ptr) {
  free(ptr);
}

int tsg_memcmp(const void* lhs, const void* rhs, size_t count) {
  return memcmp(lhs, rhs, count);
}

void* tsg_memcpy(void* dst, const void* src, size_t count) {
  return memcpy(dst, src, count);
}
void* tsg_memset(void* dst, int ch, size_t count) {
  return memset(dst, ch, count);
}

size_t tsg_strlen(const char* str) {
  return strlen(str);
}

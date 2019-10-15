/*--------------------------------------- vi: set ft=c ts=2 sw=2 et: --*-c-*--*/
/**
 * @file dummy.c
 *
 ** --------------------------------------------------------------------------*/

#include <tsugu/core/platform.h>

void* tsg_malloc(size_t size) {
  (void)size;
  return NULL;
}

void tsg_free(void* ptr) {
  (void)ptr;
  return;
}

int tsg_memcmp(const void* lhs, const void* rhs, size_t count) {
  (void)lhs;
  (void)rhs;
  (void)count;
  return 0;
}

void* tsg_memcpy(void* dst, const void* src, size_t count) {
  (void)dst;
  (void)src;
  (void)count;
  return NULL;
}

void* tsg_memset(void* dst, int ch, size_t count) {
  (void)dst;
  (void)ch;
  (void)count;
  return NULL;
}

size_t tsg_strlen(const char* str) {
  (void)str;
  return 0;
}

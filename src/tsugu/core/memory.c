/*--------------------------------------- vi: set ft=c ts=2 sw=2 et: --*-c-*--*/
/**
 * @file memory.c
 *
 ** --------------------------------------------------------------------------*/

#include <tsugu/core/memory.h>
#include <stdlib.h>

void* tsg_malloc(size_t size) {
  return malloc(size);
}

void tsg_free(void* ptr) {
  free(ptr);
}

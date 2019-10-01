
#include <tsugu/core/memory.h>
#include <stdlib.h>

void* tsg_malloc(size_t size) {
  return malloc(size);
}

void tsg_free(void* ptr) {
  free(ptr);
}

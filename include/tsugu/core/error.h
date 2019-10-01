
#ifndef TSUGU_CORE_ERROR_H
#define TSUGU_CORE_ERROR_H

#include <tsugu/core/token.h>
#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct tsg_error_s tsg_error_t;
struct tsg_error_s {
  tsg_source_range_t loc;
  char* message;
  tsg_error_t* next;
};

typedef struct tsg_errlist_s tsg_errlist_t;
struct tsg_errlist_s {
  tsg_error_t* head;
  tsg_error_t* tail;
};

void tsg_errlist_init(tsg_errlist_t* errlist);
void tsg_errlist_release(tsg_errlist_t* errlist);
void tsg_error(tsg_errlist_t* errlist, const tsg_source_range_t* loc,
               const char* format, ...);
void tsg_errorv(tsg_errlist_t* errlist, const tsg_source_range_t* loc,
                const char* format, va_list args);

#ifdef __cplusplus
}
#endif

#endif

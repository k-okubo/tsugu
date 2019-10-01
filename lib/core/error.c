
#include <tsugu/core/ast.h>
#include <tsugu/core/error.h>
#include <tsugu/core/memory.h>
#include <stdarg.h>
#include <string.h>

#define ERROR_MESSAGE_MAXLEN (2048)

static char* create_error_message(const char* format, va_list args);

void tsg_errlist_init(tsg_errlist_t* errlist) {
  errlist->head = NULL;
  errlist->tail = NULL;
}

void tsg_errlist_release(tsg_errlist_t* errlist) {
  tsg_error_t* error = errlist->head;
  while (error) {
    tsg_error_t* next = error->next;
    tsg_free(error->message);
    tsg_free(error);
    error = next;
  }

  tsg_errlist_init(errlist);
}

void tsg_error(tsg_errlist_t* errlist, const tsg_source_range_t* loc,
               const char* format, ...) {
  va_list args;
  va_start(args, format);
  tsg_errorv(errlist, loc, format, args);
  va_end(args);
}

void tsg_errorv(tsg_errlist_t* errlist, const tsg_source_range_t* loc,
                const char* format, va_list args) {
  tsg_error_t* error = tsg_malloc_obj(tsg_error_t);
  error->loc = *loc;
  error->message = create_error_message(format, args);
  error->next = NULL;

  if (errlist->head == NULL) {
    errlist->head = error;
    errlist->tail = error;
  } else {
    errlist->tail->next = error;
    errlist->tail = error;
  }
}

size_t size_min(size_t size1, size_t size2) {
  if (size1 < size2) {
    return size1;
  } else {
    return size2;
  }
}

char* create_error_message(const char* format, va_list args) {
  char buffer[ERROR_MESSAGE_MAXLEN];
  char* out = buffer;
  char* buffer_end = buffer + ERROR_MESSAGE_MAXLEN - 1;
  const char* src = format;

  while (*src && out <= buffer_end - 2) {
    if (*src == '%') {
      src++;

      if (*src == 'I') {
        tsg_ident_t* ident = va_arg(args, tsg_ident_t*);
        size_t nbytes = size_min(ident->nbytes, buffer_end - out);
        memcpy(out, ident->buffer, nbytes);
        out += nbytes;
        src++;
      } else if (*src == 's') {
        const char* string = va_arg(args, const char*);
        size_t nbytes = size_min(strlen(string), buffer_end - out);
        memcpy(out, string, nbytes);
        out += nbytes;
        src++;
      } else {
        *(out++) = '%';
        *(out++) = *(src++);
      }
    } else {
      *(out++) = *(src++);
    }
  }
  *(out++) = '\0';

  size_t message_size = out - buffer;
  char* message = tsg_malloc_arr(char, message_size);
  memcpy(message, buffer, message_size);

  return message;
}


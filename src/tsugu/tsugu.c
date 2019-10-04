
#include <tsugu/core/parser.h>
#include <tsugu/core/resolver.h>
#include <tsugu/core/scanner.h>
#include <tsugu/core/verifier.h>
#include <tsugu/engine/engine.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static bool read_source(FILE* fp, uint8_t** outbuf, size_t* outsize) {
  size_t read_size = 0;
  size_t buf_size = 4096;
  uint8_t* buffer = (uint8_t*)malloc(buf_size);
  uint8_t* buf_end = buffer + buf_size;
  uint8_t* ptr = buffer;

  while (true) {
    size_t s = fread(ptr, 1, 4096, fp);
    read_size += s;
    ptr += s;

    if (s < 4096) {
      break;
    }

    if (ptr >= buf_end) {
      buf_size *= 2;
      buffer = realloc(buffer, buf_size);
      buf_end = buffer + buf_size;
      ptr = buffer + read_size;
    }
  }
  *ptr = '\0';

  *outbuf = buffer;
  *outsize = read_size;

  return true;
}

static void print_error(tsg_error_t* error) {
  fprintf(stderr, "%" PRIi32 ":%" PRIi32 ": %s\n", error->loc.begin.line,
          error->loc.begin.column, error->message);
}

void print_errors(tsg_errlist_t* errors) {
  tsg_error_t* error = errors->head;
  while (error) {
    print_error(error);
    error = error->next;
  }
}

int main(void) {
  uint8_t* buffer;
  size_t source_size;
  read_source(stdin, &buffer, &source_size);

  tsg_scanner_t* scanner = tsg_scanner_create(buffer, source_size);
  tsg_parser_t* parser = tsg_parser_create(scanner);
  tsg_errlist_t errors;

  tsg_ast_t* ast = tsg_parser_parse(parser);
  tsg_parser_error(parser, &errors);

  if (errors.head) {
    print_errors(&errors);
    return 1;
  }

  tsg_parser_destroy(parser);
  tsg_scanner_destroy(scanner);
  free(buffer);

  tsg_resolver_t* resolver = tsg_resolver_create();
  if (tsg_resolver_resolve(resolver, ast) == false) {
    tsg_resolver_error(resolver, &errors);
    print_errors(&errors);
    return 1;
  }

  tsg_verifier_t* verifier = tsg_verifier_create();
  if (tsg_verifier_verify(verifier, ast) == false) {
    tsg_verifier_error(verifier, &errors);
    print_errors(&errors);
    return 1;
  }
  printf("syntax ok\n");

  tsg_verifier_destroy(verifier);
  tsg_resolver_destroy(resolver);

  int32_t ret = tsg_engine_run_ast(ast);
  printf("result = %" PRIi32 "\n", ret);

  tsg_ast_destroy(ast);

  return 0;
}

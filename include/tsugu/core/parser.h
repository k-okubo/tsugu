
#ifndef TSUGU_CORE_PARSER_H
#define TSUGU_CORE_PARSER_H

#include <tsugu/core/ast.h>
#include <tsugu/core/error.h>
#include <tsugu/core/scanner.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct tsg_parser_s tsg_parser_t;

tsg_parser_t* tsg_parser_create(tsg_scanner_t* scanner);
void tsg_parser_destroy(tsg_parser_t* parser);

tsg_ast_t* tsg_parser_parse(tsg_parser_t* parser);
void tsg_parser_error(const tsg_parser_t* parser, tsg_errlist_t* errors);

#ifdef __cplusplus
}
#endif

#endif

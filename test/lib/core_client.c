/*--------------------------------------- vi: set ft=c ts=2 sw=2 et: --*-c-*--*/
/**
 * @file core_client.c
 *
 ** --------------------------------------------------------------------------*/

#include <tsugu/core/parser.h>
#include <tsugu/core/resolver.h>
#include <tsugu/core/scanner.h>
#include <tsugu/core/verifier.h>

void mcount(unsigned long from, unsigned long self) {
  (void)from;
  (void)self;
  return;
}

void _start(void) {
  tsg_scanner_t* scanner = tsg_scanner_create("", 0);
  tsg_parser_t* parser = tsg_parser_create(scanner);
  tsg_ast_t* ast = tsg_parser_parse(parser);

  tsg_parser_destroy(parser);
  tsg_scanner_destroy(scanner);
  /*/
    tsg_resolver_t* resolver = tsg_resolver_create();
    tsg_resolver_resolve(resolver, ast);
    tsg_resolver_destroy(resolver);

    tsg_verifier_t* verifier = tsg_verifier_create();
    tsg_tyenv_t* tyenv = NULL;
    tsg_verifier_verify(verifier, ast, &tyenv);

    tsg_tyenv_destroy(tyenv);
    */
  tsg_ast_destroy(ast);
}

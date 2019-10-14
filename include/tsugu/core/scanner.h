/*--------------------------------------- vi: set ft=c ts=2 sw=2 et: --*-c-*--*/
/**
 * @file scanner.h
 *
 ** --------------------------------------------------------------------------*/

#ifndef TSUGU_CORE_SCANNER_H
#define TSUGU_CORE_SCANNER_H

#include <tsugu/core/token.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct tsg_scanner_s tsg_scanner_t;

tsg_scanner_t* tsg_scanner_create(const void* buffer, size_t nbytes);
void tsg_scanner_destroy(tsg_scanner_t* scanner);

void tsg_scanner_scan(tsg_scanner_t* scanner, tsg_token_t* token);

#ifdef __cplusplus
}
#endif

#endif

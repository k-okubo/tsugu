/*--------------------------------------- vi: set ft=c ts=2 sw=2 et: --*-c-*--*/
/**
 * @file token.h
 *
 ** --------------------------------------------------------------------------*/

#ifndef TSUGU_CORE_TOKEN_H
#define TSUGU_CORE_TOKEN_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct tsg_source_position_s tsg_source_position_t;
struct tsg_source_position_s {
  int32_t line;
  int32_t column;
};

typedef struct tsg_source_range_s tsg_source_range_t;
struct tsg_source_range_s {
  tsg_source_position_t begin;
  tsg_source_position_t end;
};

typedef struct tsg_byteseq_s tsg_byteseq_t;
struct tsg_byteseq_s {
  const uint8_t* buffer;
  size_t nbytes;
};

typedef enum {
  TSG_TOKEN_EOF,
  TSG_TOKEN_ADD,
  TSG_TOKEN_SUB,
  TSG_TOKEN_MUL,
  TSG_TOKEN_DIV,
  TSG_TOKEN_ASSIGN,
  TSG_TOKEN_EQ,
  TSG_TOKEN_LT,
  TSG_TOKEN_GT,
  TSG_TOKEN_LPAREN,
  TSG_TOKEN_RPAREN,
  TSG_TOKEN_LBRACE,
  TSG_TOKEN_RBRACE,
  TSG_TOKEN_COMMA,
  TSG_TOKEN_SEMICOLON,
  TSG_TOKEN_NUMBER,
  TSG_TOKEN_IDENT,
  TSG_TOKEN_DEF,
  TSG_TOKEN_VAL,
  TSG_TOKEN_IF,
  TSG_TOKEN_ELSE,
  TSG_TOKEN_ERROR,
} tsg_token_kind_t;

typedef struct tsg_token_s tsg_token_t;
struct tsg_token_s {
  tsg_token_kind_t kind;
  tsg_byteseq_t value;
  tsg_source_range_t loc;
};

const char* tsg_token_cstr(tsg_token_kind_t kind);

#ifdef __cplusplus
}
#endif

#endif

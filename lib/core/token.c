/*--------------------------------------- vi: set ft=c ts=2 sw=2 et: --*-c-*--*/
/**
 * @file token.c
 *
 ** --------------------------------------------------------------------------*/

#include <tsugu/core/token.h>

const char* tsg_token_cstr(tsg_token_kind_t kind) {
  switch (kind) {
    case TSG_TOKEN_EOF:
      return "<EOF>";

    case TSG_TOKEN_ADD:
      return "+";

    case TSG_TOKEN_SUB:
      return "-";

    case TSG_TOKEN_MUL:
      return "*";

    case TSG_TOKEN_DIV:
      return "/";

    case TSG_TOKEN_ASSIGN:
      return "=";

    case TSG_TOKEN_EQ:
      return "==";

    case TSG_TOKEN_LT:
      return "<";

    case TSG_TOKEN_GT:
      return ">";

    case TSG_TOKEN_LPAREN:
      return "(";

    case TSG_TOKEN_RPAREN:
      return ")";

    case TSG_TOKEN_LBRACE:
      return "{";

    case TSG_TOKEN_RBRACE:
      return "}";

    case TSG_TOKEN_COMMA:
      return ",";

    case TSG_TOKEN_SEMICOLON:
      return ";";

    case TSG_TOKEN_NUMBER:
      return "<NUMBER>";

    case TSG_TOKEN_IDENT:
      return "<IDENTIFIER>";

    case TSG_TOKEN_DEF:
      return "def";

    case TSG_TOKEN_VAL:
      return "val";

    case TSG_TOKEN_IF:
      return "if";

    case TSG_TOKEN_ELSE:
      return "else";

    case TSG_TOKEN_ERROR:
      return "<ERROR>";
  }

  return "";
}

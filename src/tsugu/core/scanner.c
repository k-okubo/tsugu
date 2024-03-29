/*--------------------------------------- vi: set ft=c ts=2 sw=2 et: --*-c-*--*/
/**
 * @file scanner.c
 *
 ** --------------------------------------------------------------------------*/

#include <tsugu/core/scanner.h>

#include <tsugu/core/memory.h>
#include <stdbool.h>
#include <stdint.h>

struct tsg_scanner_s {
  const uint8_t* ptr;
  const uint8_t* end;
  uint8_t ch;
  tsg_source_position_t pos;
};

static uint8_t next(tsg_scanner_t* scanner);
static uint8_t read(tsg_scanner_t* scanner);
static void skip_whitespace(tsg_scanner_t* scanner);
static void scan_number(tsg_scanner_t* scanner);
static void scan_identifier(tsg_scanner_t* scanner);
static void scan_line(tsg_scanner_t* scanner);

tsg_scanner_t* tsg_scanner_create(const void* buffer, size_t nbytes) {
  tsg_scanner_t* scanner = tsg_malloc_obj(tsg_scanner_t);
  if (scanner == NULL) {
    return NULL;
  }

  scanner->ptr = (const uint8_t*)buffer;
  scanner->end = (const uint8_t*)buffer + nbytes;
  scanner->ch = 0x00;

  scanner->pos.line = 1;
  scanner->pos.column = 1;

  read(scanner);
  return scanner;
}

void tsg_scanner_destroy(tsg_scanner_t* scanner) {
  tsg_free(scanner);
}

uint8_t next(tsg_scanner_t* scanner) {
  if (scanner->ch == '\n') {
    scanner->pos.line += 1;
    scanner->pos.column = 1;
  } else {
    scanner->pos.column += 1;
  }

  if (scanner->ptr < scanner->end) {
    scanner->ptr += 1;
  }

  return read(scanner);
}

uint8_t read(tsg_scanner_t* scanner) {
  if (scanner->ptr < scanner->end) {
    scanner->ch = *(scanner->ptr);
  } else {
    scanner->ch = 0x00;
  }

  return scanner->ch;
}

void tsg_scanner_scan(tsg_scanner_t* scanner, tsg_token_t* token) {
  skip_whitespace(scanner);

  token->loc.begin = scanner->pos;
  token->value.buffer = scanner->ptr;
  char ch = scanner->ch;

  if (ch == 0x00) {
    token->kind = TSG_TOKEN_EOF;
  } else if (ch == '+') {
    next(scanner);
    token->kind = TSG_TOKEN_ADD;
  } else if (ch == '-') {
    next(scanner);
    token->kind = TSG_TOKEN_SUB;
  } else if (ch == '*') {
    next(scanner);
    token->kind = TSG_TOKEN_MUL;
  } else if (ch == '/') {
    next(scanner);
    ch = scanner->ch;
    if (ch == '/') {
      scan_line(scanner);
      tsg_scanner_scan(scanner, token);
      return;
    } else {
      token->kind = TSG_TOKEN_DIV;
    }
  } else if (ch == '=') {
    next(scanner);
    ch = scanner->ch;
    if (ch == '=') {
      next(scanner);
      token->kind = TSG_TOKEN_EQ;
    } else {
      token->kind = TSG_TOKEN_ASSIGN;
    }
  } else if (ch == '<') {
    next(scanner);
    token->kind = TSG_TOKEN_LT;
  } else if (ch == '>') {
    next(scanner);
    token->kind = TSG_TOKEN_GT;
  } else if (ch == '(') {
    next(scanner);
    token->kind = TSG_TOKEN_LPAREN;
  } else if (ch == ')') {
    next(scanner);
    token->kind = TSG_TOKEN_RPAREN;
  } else if (ch == '{') {
    next(scanner);
    token->kind = TSG_TOKEN_LBRACE;
  } else if (ch == '}') {
    next(scanner);
    token->kind = TSG_TOKEN_RBRACE;
  } else if (ch == ',') {
    next(scanner);
    token->kind = TSG_TOKEN_COMMA;
  } else if (ch == ';') {
    next(scanner);
    token->kind = TSG_TOKEN_SEMICOLON;
  } else if ('0' <= ch && ch <= '9') {
    scan_number(scanner);
    token->kind = TSG_TOKEN_NUMBER;
  } else if (('a' <= ch && ch <= 'z') || ('A' <= ch && ch <= 'Z')) {
    scan_identifier(scanner);
    token->kind = TSG_TOKEN_IDENT;
  } else {
    token->kind = TSG_TOKEN_ERROR;
  }

  token->loc.end = scanner->pos;
  token->value.nbytes = scanner->ptr - token->value.buffer;

  if (token->kind == TSG_TOKEN_IDENT) {
    const uint8_t* buffer = token->value.buffer;
    size_t nbytes = token->value.nbytes;

    if (nbytes == 3 && tsg_memcmp(buffer, "def", 3) == 0) {
      token->kind = TSG_TOKEN_DEF;
    } else if (nbytes == 3 && tsg_memcmp(buffer, "val", 3) == 0) {
      token->kind = TSG_TOKEN_VAL;
    } else if (nbytes == 2 && tsg_memcmp(buffer, "if", 2) == 0) {
      token->kind = TSG_TOKEN_IF;
    } else if (nbytes == 4 && tsg_memcmp(buffer, "else", 4) == 0) {
      token->kind = TSG_TOKEN_ELSE;
    }
  }
}

void skip_whitespace(tsg_scanner_t* scanner) {
  uint8_t ch = scanner->ch;
  while (ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n') {
    ch = next(scanner);
  }
}

void scan_number(tsg_scanner_t* scanner) {
  uint8_t ch = scanner->ch;
  while ('0' <= ch && ch <= '9') {
    ch = next(scanner);
  }
}

void scan_identifier(tsg_scanner_t* scanner) {
  uint8_t ch = scanner->ch;
  while (('0' <= ch && ch <= '9') || ('a' <= ch && ch <= 'z') ||
         ('A' <= ch && ch <= 'Z') || ch == '_') {
    ch = next(scanner);
  }
}

void scan_line(tsg_scanner_t* scanner) {
  uint8_t ch = scanner->ch;
  while (ch != 0x00 && ch != '\n') {
    ch = next(scanner);
  }
  next(scanner);
}

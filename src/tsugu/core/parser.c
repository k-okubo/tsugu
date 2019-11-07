/*--------------------------------------- vi: set ft=c ts=2 sw=2 et: --*-c-*--*/
/**
 * @file parser.c
 *
 ** --------------------------------------------------------------------------*/

#include <tsugu/core/parser.h>

#include <tsugu/core/error.h>
#include <tsugu/core/memory.h>
#include <stdarg.h>
#include <stdbool.h>

struct tsg_parser_s {
  tsg_scanner_t* scanner;
  tsg_token_t token;
  tsg_errlist_t errors;
  int32_t last_line;
  int32_t last_error_line;
};

static void next(tsg_parser_t* parser);
static bool accept(tsg_parser_t* parsre, tsg_token_kind_t token_kind);
static bool expect(tsg_parser_t* parser, tsg_token_kind_t token_kind);
static void error(tsg_parser_t* parser, const char* format, ...);
static int_fast8_t token_prec(tsg_token_kind_t token_kind);

static tsg_block_t* parse_block(tsg_parser_t* parser);
static tsg_func_node_t* parse_func_node(tsg_parser_t* parser,
                                        tsg_func_list_t* list,
                                        tsg_func_node_t* tail);
static tsg_stmt_node_t* parse_stmt_node(tsg_parser_t* parser,
                                        tsg_stmt_list_t* list,
                                        tsg_stmt_node_t* tail);
static tsg_func_t* parse_func(tsg_parser_t* parser);
static tsg_stmt_t* parse_stmt(tsg_parser_t* parser);
static tsg_stmt_t* parse_stmt_val(tsg_parser_t* parser);
static tsg_stmt_t* parse_stmt_expr(tsg_parser_t* parser);

static tsg_expr_t* parse_expr(tsg_parser_t* parser);
static tsg_expr_t* parse_expr_binary(tsg_parser_t* parser, int lowest_prec);
static tsg_expr_t* parse_expr_primary(tsg_parser_t* parser);
static tsg_expr_t* parse_expr_call(tsg_parser_t* parser, tsg_expr_t* operand);
static tsg_expr_t* parse_expr_operand(tsg_parser_t* parser);
static tsg_expr_t* parse_expr_paren(tsg_parser_t* parser);
static tsg_expr_t* parse_expr_ifelse(tsg_parser_t* parser);
static tsg_expr_t* parse_expr_ident(tsg_parser_t* parser);
static tsg_expr_t* parse_expr_number(tsg_parser_t* parser);
static tsg_expr_list_t* parse_expr_list(tsg_parser_t* parser);

static tsg_decl_t* parse_decl(tsg_parser_t* parser);
static tsg_decl_list_t* parse_decl_list(tsg_parser_t* parser);
static tsg_ident_t* parse_ident(tsg_parser_t* parser);

tsg_parser_t* tsg_parser_create(tsg_scanner_t* scanner) {
  tsg_parser_t* parser = tsg_malloc_obj(tsg_parser_t);
  if (parser == NULL) {
    return NULL;
  }

  parser->scanner = scanner;
  tsg_errlist_init(&(parser->errors));
  parser->last_error_line = 0;

  next(parser);

  return parser;
}

void tsg_parser_destroy(tsg_parser_t* parser) {
  tsg_errlist_release(&(parser->errors));
  tsg_free(parser);
}

void tsg_parser_error(const tsg_parser_t* parser, tsg_errlist_t* errors) {
  *errors = parser->errors;
}

void next(tsg_parser_t* parser) {
  parser->last_line = parser->token.loc.end.line;
  tsg_scanner_scan(parser->scanner, &(parser->token));
}

bool accept(tsg_parser_t* parser, tsg_token_kind_t token_kind) {
  if (parser->token.kind == token_kind) {
    next(parser);
    return true;
  } else {
    return false;
  }
}

bool expect(tsg_parser_t* parser, tsg_token_kind_t token_kind) {
  bool ret = accept(parser, token_kind);
  if (ret == false) {
    error(parser, "expected '%s', found '%s'", tsg_token_cstr(token_kind),
          tsg_token_cstr(parser->token.kind));
  }
  return ret;
}

void error(tsg_parser_t* parser, const char* format, ...) {
  if (parser->token.loc.begin.line == parser->last_error_line) {
    return;
  }

  va_list args;
  va_start(args, format);
  tsg_errorv(&(parser->errors), &(parser->token.loc), format, args);
  va_end(args);

  parser->last_error_line = parser->token.loc.begin.line;
}

int_fast8_t token_prec(tsg_token_kind_t token_kind) {
  switch (token_kind) {
    case TSG_TOKEN_EQ:
    case TSG_TOKEN_LT:
    case TSG_TOKEN_GT:
      return 1;
    case TSG_TOKEN_ADD:
    case TSG_TOKEN_SUB:
      return 2;
    case TSG_TOKEN_MUL:
    case TSG_TOKEN_DIV:
      return 3;
    default:
      return -1;
  }
}

tsg_ast_t* tsg_parser_parse(tsg_parser_t* parser) {
  tsg_ast_t* ast = tsg_ast_create();
  tsg_func_t* root_func = tsg_func_create();
  tsg_decl_t* root_decl = tsg_decl_create();
  tsg_ident_t* root_name = tsg_ident_create();

  ast->root = root_func;
  root_decl->name = root_name;
  root_name->buffer = tsg_malloc_arr(uint8_t, 6);
  root_name->nbytes = 6;
  tsg_memcpy(root_name->buffer, "$main", 6);

  root_func->decl = root_decl;
  root_func->params = tsg_decl_list_create();
  root_func->body = parse_block(parser);

  expect(parser, TSG_TOKEN_EOF);

  return ast;
}

tsg_block_t* parse_block(tsg_parser_t* parser) {
  tsg_block_t* block = tsg_block_create();
  block->funcs = tsg_func_list_create();
  block->stmts = tsg_stmt_list_create();

  tsg_func_node_t* func_tail = NULL;
  tsg_stmt_node_t* stmt_tail = NULL;

  while (true) {
    tsg_func_node_t* last_func = func_tail;
    tsg_stmt_node_t* last_stmt = stmt_tail;

    switch (parser->token.kind) {
      case TSG_TOKEN_DEF:
        last_func = parse_func_node(parser, block->funcs, func_tail);
        if (last_func == func_tail) {
          goto EXIT;
        } else {
          func_tail = last_func;
        }
        break;

      default:
        last_stmt = parse_stmt_node(parser, block->stmts, stmt_tail);
        if (last_stmt == stmt_tail) {
          goto EXIT;
        } else {
          stmt_tail = last_stmt;
        }
        break;
    }
  }

EXIT:
  return block;
}

tsg_func_node_t* parse_func_node(tsg_parser_t* parser, tsg_func_list_t* list,
                                 tsg_func_node_t* tail) {
  tsg_func_t* func = parse_func(parser);
  if (func == NULL) {
    return tail;
  }

  tsg_func_node_t* node = tsg_func_node_create();
  node->func = func;

  if (tail == NULL) {
    list->head = node;
  } else {
    tail->next = node;
  }

  list->size += 1;
  return node;
}

tsg_stmt_node_t* parse_stmt_node(tsg_parser_t* parser, tsg_stmt_list_t* list,
                                 tsg_stmt_node_t* tail) {
  tsg_stmt_t* stmt = parse_stmt(parser);
  if (stmt == NULL) {
    return tail;
  }

  tsg_stmt_node_t* node = tsg_stmt_node_create();
  node->stmt = stmt;

  if (tail == NULL) {
    list->head = node;
  } else {
    tail->next = node;
  }

  list->size += 1;
  return node;
}

tsg_func_t* parse_func(tsg_parser_t* parser) {
  if (!accept(parser, TSG_TOKEN_DEF)) {
    return NULL;
  }

  tsg_func_t* func = tsg_func_create();
  func->decl = tsg_decl_create();

  tsg_ident_t* name = parse_ident(parser);
  if (name == NULL) {
    error(parser, "expected identifier");
  }

  func->decl->name = name;
  expect(parser, TSG_TOKEN_LPAREN);
  func->params = parse_decl_list(parser);
  expect(parser, TSG_TOKEN_RPAREN);

  expect(parser, TSG_TOKEN_LBRACE);
  func->body = parse_block(parser);
  expect(parser, TSG_TOKEN_RBRACE);

  return func;
}

tsg_stmt_t* parse_stmt(tsg_parser_t* parser) {
  tsg_stmt_t* stmt = NULL;

  switch (parser->token.kind) {
    case TSG_TOKEN_VAL:
      stmt = parse_stmt_val(parser);
      break;

    default:
      stmt = parse_stmt_expr(parser);
      break;
  }

  if (accept(parser, TSG_TOKEN_SEMICOLON) == false) {
    if (parser->token.kind != TSG_TOKEN_EOF &&
        parser->token.kind != TSG_TOKEN_RBRACE) {
      if (parser->token.loc.begin.line == parser->last_line) {
        error(parser, "expected '%s', found '%s'",
              tsg_token_cstr(TSG_TOKEN_SEMICOLON),
              tsg_token_cstr(parser->token.kind));
      }
    }
  }

  return stmt;
}

tsg_stmt_t* parse_stmt_val(tsg_parser_t* parser) {
  if (!accept(parser, TSG_TOKEN_VAL)) {
    return NULL;
  }

  tsg_stmt_t* stmt = tsg_stmt_create(TSG_STMT_VAL);

  stmt->val.decl = parse_decl(parser);
  if (stmt->val.decl == NULL) {
    error(parser, "expected declare");
  }

  expect(parser, TSG_TOKEN_ASSIGN);

  stmt->val.expr = parse_expr(parser);
  if (stmt->val.expr == NULL) {
    error(parser, "expected expression");
  }

  return stmt;
}

tsg_stmt_t* parse_stmt_expr(tsg_parser_t* parser) {
  tsg_expr_t* expr = parse_expr(parser);
  if (expr == NULL) {
    return NULL;
  }

  tsg_stmt_t* stmt = tsg_stmt_create(TSG_STMT_EXPR);
  stmt->expr.expr = expr;

  return stmt;
}

tsg_expr_t* parse_expr(tsg_parser_t* parser) {
  return parse_expr_binary(parser, 0);
}

tsg_expr_t* parse_expr_binary(tsg_parser_t* parser, int lowest_prec) {
  tsg_expr_t* lhs = parse_expr_primary(parser);
  if (lhs == NULL) {
    return NULL;
  }

  while (1) {
    int_fast8_t prec = token_prec(parser->token.kind);
    if (prec <= lowest_prec) {
      return lhs;
    }

    tsg_token_kind_t op = parser->token.kind;
    next(parser);

    tsg_expr_t* rhs = parse_expr_binary(parser, prec);
    if (rhs == NULL) {
      error(parser, "expected expression");
    } else {
      tsg_expr_t* expr = tsg_expr_create(TSG_EXPR_BINARY);
      expr->loc.begin = lhs->loc.begin;
      expr->loc.end = rhs->loc.end;
      expr->binary.op = op;
      expr->binary.lhs = lhs;
      expr->binary.rhs = rhs;

      lhs = expr;
    }
  }
}

tsg_expr_t* parse_expr_primary(tsg_parser_t* parser) {
  tsg_expr_t* operand = parse_expr_operand(parser);
  if (operand == NULL) {
    return NULL;
  }

  while (true) {
    switch (parser->token.kind) {
      case TSG_TOKEN_LPAREN:
        operand = parse_expr_call(parser, operand);
        break;

      default:
        return operand;
    }
  }
}

tsg_expr_t* parse_expr_call(tsg_parser_t* parser, tsg_expr_t* operand) {
  if (!accept(parser, TSG_TOKEN_LPAREN)) {
    return NULL;
  }

  tsg_expr_list_t* args = parse_expr_list(parser);
  tsg_source_position_t end = parser->token.loc.end;
  expect(parser, TSG_TOKEN_RPAREN);

  tsg_expr_t* expr = tsg_expr_create(TSG_EXPR_CALL);
  expr->loc.begin = operand->loc.begin;
  expr->loc.end = end;
  expr->call.callee = operand;
  expr->call.args = args;

  return expr;
}

tsg_expr_t* parse_expr_operand(tsg_parser_t* parser) {
  switch (parser->token.kind) {
    case TSG_TOKEN_LPAREN:
      return parse_expr_paren(parser);

    case TSG_TOKEN_IF:
      return parse_expr_ifelse(parser);

    case TSG_TOKEN_IDENT:
      return parse_expr_ident(parser);

    case TSG_TOKEN_NUMBER:
      return parse_expr_number(parser);

    default:
      return NULL;
  }
}

tsg_expr_t* parse_expr_paren(tsg_parser_t* parser) {
  tsg_source_position_t begin = parser->token.loc.begin;
  if (!accept(parser, TSG_TOKEN_LPAREN)) {
    return NULL;
  }

  tsg_expr_t* expr = parse_expr(parser);
  expr->loc.begin = begin;
  expr->loc.end = parser->token.loc.end;
  expect(parser, TSG_TOKEN_RPAREN);

  return expr;
}

tsg_expr_t* parse_expr_ifelse(tsg_parser_t* parser) {
  tsg_source_position_t begin = parser->token.loc.begin;
  if (!accept(parser, TSG_TOKEN_IF)) {
    return NULL;
  }

  expect(parser, TSG_TOKEN_LPAREN);
  tsg_expr_t* cond = parse_expr(parser);
  if (cond == NULL) {
    error(parser, "expected expression");
  }
  expect(parser, TSG_TOKEN_RPAREN);

  expect(parser, TSG_TOKEN_LBRACE);
  tsg_block_t* thn = parse_block(parser);
  if (thn->stmts->size == 0) {
    error(parser, "block is empty");
  }

  expect(parser, TSG_TOKEN_RBRACE);
  expect(parser, TSG_TOKEN_ELSE);
  expect(parser, TSG_TOKEN_LBRACE);

  tsg_block_t* els = parse_block(parser);
  if (els->stmts->size == 0) {
    error(parser, "block is empty");
  }

  tsg_source_position_t end = parser->token.loc.end;
  expect(parser, TSG_TOKEN_RBRACE);

  tsg_expr_t* expr = tsg_expr_create(TSG_EXPR_IFELSE);
  expr->loc.begin = begin;
  expr->loc.end = end;
  expr->ifelse.cond = cond;
  expr->ifelse.thn = thn;
  expr->ifelse.els = els;

  return expr;
}

tsg_expr_t* parse_expr_ident(tsg_parser_t* parser) {
  tsg_ident_t* ident = parse_ident(parser);
  if (ident == NULL) {
    return NULL;
  }

  tsg_expr_t* expr = tsg_expr_create(TSG_EXPR_IDENT);
  expr->loc = ident->loc;
  expr->ident.name = ident;
  expr->ident.object = NULL;

  return expr;
}

tsg_expr_t* parse_expr_number(tsg_parser_t* parser) {
  if (parser->token.kind != TSG_TOKEN_NUMBER) {
    return NULL;
  }

  int32_t number = 0;
  const uint8_t* ptr = parser->token.value.buffer;
  const uint8_t* end = ptr + parser->token.value.nbytes;

  while (ptr < end) {
    number = number * 10 + (*ptr - '0');
    ptr++;
  }

  tsg_expr_t* expr = tsg_expr_create(TSG_EXPR_NUMBER);
  expr->loc = parser->token.loc;
  expr->number.value = number;
  next(parser);

  return expr;
}

tsg_expr_list_t* parse_expr_list(tsg_parser_t* parser) {
  tsg_expr_list_t* result = tsg_expr_list_create();

  tsg_expr_node_t* last = NULL;
  while (true) {
    tsg_expr_t* expr = parse_expr(parser);
    if (expr == NULL) {
      if (last != NULL) {
        error(parser, "expected expression");
      }
      break;
    }

    tsg_expr_node_t* node = tsg_expr_node_create();
    node->expr = expr;

    if (last == NULL) {
      result->head = node;
    } else {
      last->next = node;
    }

    result->size += 1;
    last = node;

    if (!accept(parser, TSG_TOKEN_COMMA)) {
      break;
    }
  }

  return result;
}

tsg_decl_t* parse_decl(tsg_parser_t* parser) {
  tsg_ident_t* ident = parse_ident(parser);
  if (ident == NULL) {
    return NULL;
  }

  tsg_decl_t* decl = tsg_decl_create();
  decl->name = ident;

  return decl;
}

tsg_decl_list_t* parse_decl_list(tsg_parser_t* parser) {
  tsg_decl_list_t* result = tsg_decl_list_create();

  tsg_decl_node_t* last = NULL;
  while (true) {
    tsg_decl_t* decl = parse_decl(parser);
    if (decl == NULL) {
      if (last != NULL) {
        error(parser, "expected declare");
      }
      break;
    }

    tsg_decl_node_t* node = tsg_decl_node_create();
    node->decl = decl;

    if (last == NULL) {
      result->head = node;
    } else {
      last->next = node;
    }

    result->size += 1;
    last = node;

    if (!accept(parser, TSG_TOKEN_COMMA)) {
      break;
    }
  }

  return result;
}

tsg_ident_t* parse_ident(tsg_parser_t* parser) {
  if (parser->token.kind != TSG_TOKEN_IDENT) {
    return NULL;
  }

  const uint8_t* src = parser->token.value.buffer;
  size_t nbytes = parser->token.value.nbytes;

  tsg_ident_t* ident = tsg_ident_create();
  ident->buffer = tsg_malloc_arr(uint8_t, nbytes + 1);
  tsg_memcpy(ident->buffer, src, nbytes);
  ident->buffer[nbytes] = 0;
  ident->nbytes = nbytes;
  ident->loc = parser->token.loc;

  next(parser);

  return ident;
}

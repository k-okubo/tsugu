
#ifndef TSUGU_CORE_AST_H
#define TSUGU_CORE_AST_H

#include <tsugu/core/token.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
  TSG_EXPR_IFELSE,
  TSG_EXPR_BINARY,
  TSG_EXPR_CALL,
  TSG_EXPR_VARIABLE,
  TSG_EXPR_NUMBER,
} tsg_expr_kind_t;

typedef enum {
  TSG_STMT_VAL,
  TSG_STMT_EXPR,
} tsg_stmt_kind_t;

typedef struct tsg_ast_s tsg_ast_t;
typedef struct tsg_func_s tsg_func_t;
typedef struct tsg_block_s tsg_block_t;
typedef struct tsg_stmt_s tsg_stmt_t;
typedef struct tsg_expr_s tsg_expr_t;
typedef struct tsg_decl_s tsg_decl_t;
typedef struct tsg_ident_s tsg_ident_t;

typedef struct tsg_func_list_s tsg_func_list_t;
typedef struct tsg_func_node_s tsg_func_node_t;
typedef struct tsg_stmt_list_s tsg_stmt_list_t;
typedef struct tsg_stmt_node_s tsg_stmt_node_t;
typedef struct tsg_expr_list_s tsg_expr_list_t;
typedef struct tsg_expr_node_s tsg_expr_node_t;
typedef struct tsg_decl_list_s tsg_decl_list_t;
typedef struct tsg_decl_node_s tsg_decl_node_t;

struct tsg_ast_s {
  tsg_func_list_t* functions;
};

struct tsg_func_s {
  tsg_decl_t* decl;
  tsg_decl_list_t* args;
  tsg_block_t* body;
  int32_t n_types;
};

struct tsg_block_s {
  tsg_stmt_list_t* stmts;
  size_t n_decls;
};

struct tsg_stmt_val_s {
  tsg_decl_t* decl;
  tsg_expr_t* expr;
};

struct tsg_stmt_expr_s {
  tsg_expr_t* expr;
};

struct tsg_stmt_s {
  tsg_stmt_kind_t kind;

  union {
    struct tsg_stmt_val_s val;
    struct tsg_stmt_expr_s expr;
  };
};

struct tsg_expr_binary_s {
  tsg_token_kind_t op;
  tsg_expr_t* lhs;
  tsg_expr_t* rhs;
};

struct tsg_expr_call_s {
  tsg_expr_t* callee;
  tsg_expr_list_t* args;
};

struct tsg_expr_ifelse_s {
  tsg_expr_t* cond;
  tsg_block_t* thn;
  tsg_block_t* els;
};

struct tsg_expr_variable_s {
  tsg_ident_t* name;
  tsg_decl_t* resolved;
};

struct tsg_expr_number_s {
  int32_t value;
};

struct tsg_expr_s {
  tsg_expr_kind_t kind;
  int32_t type_id;
  tsg_source_range_t loc;

  union {
    struct tsg_expr_binary_s binary;
    struct tsg_expr_call_s call;
    struct tsg_expr_ifelse_s ifelse;
    struct tsg_expr_variable_s variable;
    struct tsg_expr_number_s number;
  };
};

struct tsg_decl_s {
  tsg_ident_t* name;
  int32_t type_id;
  int32_t depth;
  int32_t index;
};

struct tsg_ident_s {
  uint8_t* buffer;
  size_t nbytes;
  tsg_source_range_t loc;
};

struct tsg_func_list_s {
  tsg_func_node_t* head;
  size_t size;
};

struct tsg_func_node_s {
  tsg_func_t* func;
  tsg_func_node_t* next;
};

struct tsg_stmt_list_s {
  tsg_stmt_node_t* head;
  size_t size;
};

struct tsg_stmt_node_s {
  tsg_stmt_t* stmt;
  tsg_stmt_node_t* next;
};

struct tsg_expr_list_s {
  tsg_expr_node_t* head;
  size_t size;
};

struct tsg_expr_node_s {
  tsg_expr_t* expr;
  tsg_expr_node_t* next;
};

struct tsg_decl_list_s {
  tsg_decl_node_t* head;
  size_t size;
};

struct tsg_decl_node_s {
  tsg_decl_t* decl;
  tsg_decl_node_t* next;
};

const char* tsg_ident_cstr(tsg_ident_t* ident);
void tsg_ast_destroy(tsg_ast_t* ast);

#ifdef __cplusplus
}
#endif

#endif

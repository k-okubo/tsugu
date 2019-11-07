/*--------------------------------------- vi: set ft=c ts=2 sw=2 et: --*-c-*--*/
/**
 * @file scope.c
 *
 ** --------------------------------------------------------------------------*/

#include <tsugu/core/scope.h>

#include <tsugu/core/memory.h>
#include <tsugu/core/symtbl.h>

struct tsg_scope_s {
  tsg_scope_t* outer;
  tsg_symtbl_t* symtbl;
};

tsg_scope_t* tsg_scope_create(tsg_scope_t* outer) {
  tsg_scope_t* scope = tsg_malloc_obj(tsg_scope_t);

  scope->outer = outer;
  scope->symtbl = tsg_symtbl_create();

  return scope;
}

void tsg_scope_destroy(tsg_scope_t* scope) {
  if (scope == NULL) {
    return;
  }

  tsg_symtbl_destroy(scope->symtbl);
  tsg_free(scope);
}

bool tsg_scope_add(tsg_scope_t* scope, tsg_ident_t* ident,
                   tsg_member_t* member) {
  tsg_assert(scope != NULL);
  tsg_assert(ident != NULL);
  tsg_assert(ident->buffer != NULL);
  tsg_assert(ident->nbytes > 0);
  tsg_assert(member != NULL);

  return tsg_symtbl_insert(scope->symtbl, ident, member);
}

tsg_member_t* tsg_scope_find(tsg_scope_t* scope, tsg_ident_t* ident) {
  tsg_assert(scope != NULL);
  tsg_assert(ident != NULL);
  tsg_assert(ident->buffer != NULL);
  tsg_assert(ident->nbytes > 0);

  while (scope != NULL) {
    tsg_member_t* member = tsg_symtbl_lookup(scope->symtbl, ident);
    if (member != NULL) {
      return member;
    }
    scope = scope->outer;
  }

  return NULL;
}

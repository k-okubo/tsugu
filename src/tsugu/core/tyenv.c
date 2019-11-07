/*--------------------------------------- vi: set ft=c ts=2 sw=2 et: --*-c-*--*/
/**
 * @file tyenv.c
 *
 ** --------------------------------------------------------------------------*/

#include <tsugu/core/tyenv.h>

#include <tsugu/core/memory.h>
#include <tsugu/core/platform.h>

static tsg_type_t** find_tyenv_entry(tsg_tyenv_t* tyenv, tsg_tyvar_t* tyvar);

tsg_tyset_t* tsg_tyset_create(tsg_tyset_t* outer) {
  tsg_tyset_t* tyset = tsg_malloc_obj(tsg_tyset_t);

  if (outer != NULL) {
    tyset->depth = outer->depth + 1;
  } else {
    tyset->depth = 0;
  }

  tyset->outer = outer;
  tsg_assert(tyset->depth >= 0);
  tyset->n_entries = 0;

  return tyset;
}

void tsg_tyset_destroy(tsg_tyset_t* tyset) {
  if (tyset == NULL) {
    return;
  }

  tsg_free(tyset);
}

tsg_tyvar_t* tsg_tyvar_create(tsg_tyset_t* tyset) {
  tsg_assert(tyset != NULL);

  tsg_tyvar_t* tyvar = tsg_malloc_obj(tsg_tyvar_t);

  tyvar->tyset = tyset;
  tyvar->index = tyset->n_entries;
  tyset->n_entries += 1;

  return tyvar;
}

void tsg_tyvar_destroy(tsg_tyvar_t* tyvar) {
  if (tyvar == NULL) {
    return;
  }

  tsg_free(tyvar);
}

tsg_tyenv_t* tsg_tyenv_create(tsg_tyset_t* tyset, tsg_tyenv_t* outer) {
  tsg_assert(tyset != NULL);
  tsg_assert((outer == NULL && tyset->outer == NULL) ||
             (outer->tyset == tyset->outer));

  tsg_tyenv_t* tyenv = tsg_malloc_obj(tsg_tyenv_t);

  tyenv->outer = outer;
  tyenv->tyset = tyset;

  if (tyset->n_entries > 0) {
    int32_t size = tyset->n_entries;
    tyenv->arr = tsg_malloc_arr(tsg_type_t*, size);
    tyenv->size = size;
    tsg_memset(tyenv->arr, 0, sizeof(tsg_tyenv_t*) * size);
  } else {
    tyenv->arr = NULL;
    tyenv->size = 0;
  }

  return tyenv;
}

void tsg_tyenv_destroy(tsg_tyenv_t* tyenv) {
  if (tyenv == NULL) {
    return;
  }

  tsg_type_t** p = tyenv->arr;
  tsg_type_t** end = p + tyenv->size;
  while (p < end) {
    if (*p != NULL) {
      tsg_type_release(*p);
    }
    p++;
  }

  tsg_free(tyenv->arr);
  tsg_free(tyenv);
}

void tsg_tyenv_set(tsg_tyenv_t* tyenv, tsg_tyvar_t* tyvar, tsg_type_t* type) {
  tsg_assert(type != NULL);
  tsg_type_t** entry = find_tyenv_entry(tyenv, tyvar);
  tsg_assert(entry != NULL);
  tsg_assert(*entry == NULL);

  tsg_type_retain(type);
  *entry = type;
}

tsg_type_t* tsg_tyenv_get(tsg_tyenv_t* tyenv, tsg_tyvar_t* tyvar) {
  tsg_type_t** entry = find_tyenv_entry(tyenv, tyvar);
  tsg_assert(entry != NULL);
  return *entry;
}

tsg_type_t** find_tyenv_entry(tsg_tyenv_t* tyenv, tsg_tyvar_t* tyvar) {
  tsg_assert(tyenv != NULL);
  tsg_assert(tyvar != NULL);

  while (tyenv && tyenv->tyset->depth >= tyvar->tyset->depth) {
    if (tyenv->tyset == tyvar->tyset) {
      tsg_assert(tyvar->index < tyenv->size);
      return tyenv->arr + tyvar->index;
    }

    tyenv = tyenv->outer;
  }

  return NULL;
}

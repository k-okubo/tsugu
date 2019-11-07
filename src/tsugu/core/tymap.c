/*--------------------------------------- vi: set ft=c ts=2 sw=2 et: --*-c-*--*/
/**
 * @file tymap.c
 *
 ** --------------------------------------------------------------------------*/

#include <tsugu/core/tymap.h>

#include <tsugu/core/memory.h>

typedef struct tsg_tyenv_entry_s tsg_tyenv_entry_t;

struct tsg_tyenv_entry_s {
  tsg_type_arr_t* key;
  tsg_tyenv_t* tyenv;
  tsg_tyenv_entry_t* next;
};

struct tsg_tymap_s {
  tsg_tyenv_entry_t* head;
};

tsg_tymap_t* tsg_tymap_create(void) {
  tsg_tymap_t* tymap = tsg_malloc_obj(tsg_tymap_t);

  tymap->head = NULL;

  return tymap;
}

void tsg_tymap_destroy(tsg_tymap_t* tymap) {
  if (tymap == NULL) {
    return;
  }

  tsg_tyenv_entry_t* entry = tymap->head;
  while (entry != NULL) {
    tsg_tyenv_entry_t* next = entry->next;
    tsg_type_arr_destroy(entry->key);
    tsg_tyenv_destroy(entry->tyenv);
    tsg_free(entry);
    entry = next;
  }

  tsg_free(tymap);
}

void tsg_tymap_add(tsg_tymap_t* tymap, tsg_type_arr_t* key, tsg_tyenv_t* env) {
  tsg_assert(tymap != NULL);
  tsg_assert(key != NULL);
  tsg_assert(env != NULL);

  tsg_tyenv_entry_t* entry = tsg_malloc_obj(tsg_tyenv_entry_t);

  entry->key = tsg_type_arr_dup(key);
  entry->tyenv = env;
  entry->next = tymap->head;
  tymap->head = entry;
}

tsg_tyenv_t* tsg_tymap_get(tsg_tymap_t* tymap, tsg_type_arr_t* key) {
  tsg_assert(tymap != NULL);
  tsg_assert(key != NULL);

  tsg_tyenv_entry_t* entry = tymap->head;
  while (entry) {
    if (tsg_type_arr_equals(key, entry->key)) {
      return entry->tyenv;
    }
    entry = entry->next;
  }
  return NULL;
}

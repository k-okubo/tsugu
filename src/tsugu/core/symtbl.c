/*--------------------------------------- vi: set ft=c ts=2 sw=2 et: --*-c-*--*/
/**
 * @file symtbl.c
 *
 ** --------------------------------------------------------------------------*/

#include <tsugu/core/symtbl.h>

#include <tsugu/core/memory.h>
#include <inttypes.h>

#define NAMETABLE_INITIAL_HASH_BITS (6)
#define NAMETABLE_LINEAR_SEARCH_LIMIT (10)

typedef struct record_s record_t;
struct record_s {
  tsg_ident_t* key;
  tsg_member_t* val;
};

struct tsg_symtbl_s {
  record_t* table;
  int_fast8_t hash_bits;
};

static void alloc_table(tsg_symtbl_t* symtbl, int_fast8_t hash_bits);
static record_t* find_record(tsg_symtbl_t* symtbl, tsg_ident_t* key);
static void rehash_table(tsg_symtbl_t* symtbl);
static bool restore_entries(record_t* src, size_t nslots, tsg_symtbl_t* dst);

static size_t table_nslots(int_fast8_t hash_bits);
static size_t table_index(int_fast8_t hash_bits, tsg_ident_t* ident);
static uint64_t hash_mask(int_fast8_t hash_bits);
static uint64_t ident_hash(tsg_ident_t* ident);
static bool same_ident(tsg_ident_t* ident1, tsg_ident_t* ident2);

tsg_symtbl_t* tsg_symtbl_create(void) {
  tsg_symtbl_t* symtbl = tsg_malloc_obj(tsg_symtbl_t);
  if (symtbl == NULL) {
    return symtbl;
  }

  alloc_table(symtbl, NAMETABLE_INITIAL_HASH_BITS);

  return symtbl;
}

void alloc_table(tsg_symtbl_t* symtbl, int_fast8_t hash_bits) {
  size_t nslots = table_nslots(hash_bits);
  symtbl->table = tsg_malloc_arr(record_t, nslots);
  symtbl->hash_bits = hash_bits;
  tsg_symtbl_clear(symtbl);
}

void tsg_symtbl_destroy(tsg_symtbl_t* symtbl) {
  tsg_free(symtbl->table);
  tsg_free(symtbl);
}

void tsg_symtbl_clear(tsg_symtbl_t* symtbl) {
  size_t nslots = table_nslots(symtbl->hash_bits);
  tsg_memset(symtbl->table, 0, sizeof(record_t) * nslots);
}

bool tsg_symtbl_insert(tsg_symtbl_t* symtbl, tsg_ident_t* ident,
                       tsg_member_t* member) {
  while (true) {
    record_t* record = find_record(symtbl, ident);

    if (record != NULL) {
      if (record->key != NULL) {
        // already exists
        return false;
      } else {
        // add new entry
        record->key = ident;
        record->val = member;
        return true;
      }
    } else {
      // table full
      rehash_table(symtbl);
    }
  }
}

tsg_member_t* tsg_symtbl_lookup(tsg_symtbl_t* symtbl, tsg_ident_t* ident) {
  record_t* record = find_record(symtbl, ident);

  if (record != NULL) {
    if (record->key != NULL) {
      return record->val;
    }
  }

  return NULL;
}

record_t* find_record(tsg_symtbl_t* symtbl, tsg_ident_t* key) {
  size_t base_idx = table_index(symtbl->hash_bits, key);
  uint64_t mask = hash_mask(symtbl->hash_bits);

  for (size_t i = 0; i < NAMETABLE_LINEAR_SEARCH_LIMIT; i++) {
    record_t* record = symtbl->table + ((base_idx + i) & mask);

    if (record->key == NULL) {
      return record;
    }
    if (same_ident(record->key, key)) {
      return record;
    }
  }

  // table full
  return NULL;
}

void rehash_table(tsg_symtbl_t* symtbl) {
  record_t* old_table = symtbl->table;
  size_t old_nslots = table_nslots(symtbl->hash_bits);

  int_fast8_t hash_bits = symtbl->hash_bits;

  while (true) {
    hash_bits += 1;
    alloc_table(symtbl, hash_bits);
    if (restore_entries(old_table, old_nslots, symtbl)) {
      break;
    }
    tsg_free(symtbl->table);
  }

  tsg_free(old_table);
}

bool restore_entries(record_t* src_rec, size_t nslots, tsg_symtbl_t* dst) {
  record_t* end = src_rec + nslots;

  while (src_rec < end) {
    if (src_rec->key != NULL) {
      record_t* dst_rec = find_record(dst, src_rec->key);
      if (dst_rec == NULL) {
        return false;
      }
      *dst_rec = *src_rec;
    }

    src_rec++;
  }

  return true;
}

size_t table_nslots(int_fast8_t hash_bits) {
  return UINT64_C(1) << hash_bits;
}

size_t table_index(int_fast8_t hash_bits, tsg_ident_t* ident) {
  return ident_hash(ident) & hash_mask(hash_bits);
}

uint64_t hash_mask(int_fast8_t hash_bits) {
  return (UINT64_C(1) << hash_bits) - 1;
}

uint64_t ident_hash(tsg_ident_t* ident) {
  uint8_t* ptr = ident->buffer;
  uint8_t* end = ptr + ident->nbytes;

  // FNV-1 hash
  uint64_t hash = UINT64_C(14695981039346656037);
  while (ptr < end) {
    hash = hash * UINT64_C(1099511628211);
    hash = hash ^ (*ptr);
    ptr++;
  }

  return hash;
}

bool same_ident(tsg_ident_t* ident1, tsg_ident_t* ident2) {
  if (ident1->nbytes != ident2->nbytes) {
    return false;
  } else {
    return tsg_memcmp(ident1->buffer, ident2->buffer, ident1->nbytes) == 0;
  }
}

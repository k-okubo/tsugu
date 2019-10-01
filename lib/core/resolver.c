
#include "resolver.h"

#include <tsugu/core/memory.h>
#include <inttypes.h>
#include <string.h>

#define NAMETABLE_INITIAL_HASH_BITS (6)
#define NAMETABLE_LINEAR_SEARCH_LIMIT (10)

typedef struct record_s record_t;
struct record_s {
  tsg_decl_t* payload;
};

struct tsg_resolver_s {
  record_t* table;
  int_fast8_t hash_bits;
};

static void alloc_table(tsg_resolver_t* resolver, int_fast8_t hash_bits);
static record_t* find_record(tsg_resolver_t* resolver, tsg_ident_t* key);
static void rehash_table(tsg_resolver_t* resolver);
static bool restore_entries(record_t* src, size_t nslots, tsg_resolver_t* dst);

static size_t table_nslots(int_fast8_t hash_bits);
static size_t table_index(int_fast8_t hash_bits, tsg_ident_t* ident);
static uint64_t hash_mask(int_fast8_t hash_bits);
static uint64_t ident_hash(tsg_ident_t* ident);
static bool same_ident(tsg_ident_t* ident1, tsg_ident_t* ident2);

tsg_resolver_t* tsg_resolver_create(void) {
  tsg_resolver_t* resolver = tsg_malloc_obj(tsg_resolver_t);
  if (resolver == NULL) {
    return resolver;
  }

  alloc_table(resolver, NAMETABLE_INITIAL_HASH_BITS);

  return resolver;
}

void alloc_table(tsg_resolver_t* resolver, int_fast8_t hash_bits) {
  size_t nslots = table_nslots(hash_bits);
  resolver->table = tsg_malloc_arr(record_t, nslots);
  resolver->hash_bits = hash_bits;
  tsg_resolver_clear(resolver);
}

void tsg_resolver_destroy(tsg_resolver_t* resolver) {
  tsg_free(resolver->table);
  tsg_free(resolver);
}

void tsg_resolver_clear(tsg_resolver_t* resolver) {
  size_t nslots = table_nslots(resolver->hash_bits);
  memset(resolver->table, 0, sizeof(record_t) * nslots);
}

bool tsg_resolver_insert(tsg_resolver_t* resolver, tsg_decl_t* decl) {
  while (true) {
    record_t* record = find_record(resolver, decl->name);

    if (record != NULL) {
      if (record->payload != NULL) {
        // already exists
        return false;
      } else {
        // add new entry
        record->payload = decl;
        return true;
      }
    } else {
      // table full
      rehash_table(resolver);
    }
  }
}

bool tsg_resolver_lookup(tsg_resolver_t* resolver, tsg_ident_t* key,
                         tsg_decl_t** outval) {
  record_t* record = find_record(resolver, key);

  if (record != NULL) {
    if (record->payload != NULL) {
      *outval = record->payload;
      return true;
    }
  }

  return false;
}

record_t* find_record(tsg_resolver_t* resolver, tsg_ident_t* key) {
  size_t base_idx = table_index(resolver->hash_bits, key);
  uint64_t mask = hash_mask(resolver->hash_bits);

  for (size_t i = 0; i < NAMETABLE_LINEAR_SEARCH_LIMIT; i++) {
    record_t* record = resolver->table + ((base_idx + i) & mask);

    if (record->payload == NULL) {
      return record;
    }
    if (same_ident(record->payload->name, key)) {
      return record;
    }
  }

  // table full
  return NULL;
}

void rehash_table(tsg_resolver_t* resolver) {
  record_t* old_table = resolver->table;
  size_t old_nslots = table_nslots(resolver->hash_bits);

  int_fast8_t hash_bits = resolver->hash_bits;

  while (true) {
    hash_bits += 1;
    alloc_table(resolver, hash_bits);
    if (restore_entries(old_table, old_nslots, resolver)) {
      break;
    }
    tsg_free(resolver->table);
  }

  tsg_free(old_table);
}

bool restore_entries(record_t* src_rec, size_t nslots, tsg_resolver_t* dst) {
  record_t* end = src_rec + nslots;

  while (src_rec < end) {
    if (src_rec->payload != NULL) {
      record_t* dst_rec = find_record(dst, src_rec->payload->name);
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
    return memcmp(ident1->buffer, ident2->buffer, ident1->nbytes) == 0;
  }
}

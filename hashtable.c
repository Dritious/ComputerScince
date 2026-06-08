#include "allocator.h"
#include <string.h>

typedef struct HashEntry {
  char *key;
  void *value;
  struct HashEntry *next;
} HashEntry;

typedef struct {
  IAllocator *alloc;
  HashEntry **buckets;
  size_t bucket_count;
  size_t size;
} HashTable;

static size_t hash_string(const char *str, size_t bucket_count) {
  size_t hash = 5381;
  for (const unsigned char *p = (const unsigned char *)str; *p; ++p) {
    hash = ((hash << 5) + hash) + *p;
  }
  return hash % bucket_count;
}

int hashtable_init(HashTable *table, IAllocator *alloc, size_t bucket_count) {
  if (table == NULL || alloc == NULL) {
    return -1;
  }
  if (bucket_count == 0) {
    bucket_count = 16;
  }

  table->alloc = alloc;
  table->bucket_count = bucket_count;
  table->size = 0;
  table->buckets = i_alloc(alloc, bucket_count * sizeof(HashEntry *));
  if (table->buckets == NULL) {
    return -1;
  }
  for (size_t i = 0; i < bucket_count; ++i) {
    table->buckets[i] = NULL;
  }
  return 0;
}

int hashtable_insert(HashTable *table, const char *key, void *value) {
  if (table == NULL || key == NULL) {
    return -1;
  }
  size_t idx = hash_string(key, table->bucket_count);

  for (HashEntry *e = table->buckets[idx]; e != NULL; e = e->next) {
    if (strcmp(e->key, key) == 0) {
      e->value = value;
      return 0;
    }
  }

  HashEntry *entry = i_alloc(table->alloc, sizeof(HashEntry));
  if (entry == NULL) {
    return -1;
  }

  size_t key_len = strlen(key) + 1;
  entry->key = i_alloc(table->alloc, key_len);
  if (entry->key == NULL) {
    i_free(table->alloc, entry);
    return -1;
  }
  memcpy(entry->key, key, key_len);
  entry->value = value;

  entry->next = table->buckets[idx];
  table->buckets[idx] = entry;
  table->size++;
  return 0;
}

int hashtable_get(const HashTable *table, const char *key, void **out) {
  if (table == NULL || key == NULL) {
    return -1;
  }
  size_t idx = hash_string(key, table->bucket_count);
  for (HashEntry *e = table->buckets[idx]; e != NULL; e = e->next) {
    if (strcmp(e->key, key) == 0) {
      if (out != NULL) {
        *out = e->value;
      }
      return 0;
    }
  }
  return -1;
}

int hashtable_remove(HashTable *table, const char *key) {
  if (table == NULL || key == NULL) {
    return -1;
  }
  size_t idx = hash_string(key, table->bucket_count);

  HashEntry *prev = NULL;
  HashEntry *cur = table->buckets[idx];
  while (cur != NULL) {
    if (strcmp(cur->key, key) == 0) {
      if (prev == NULL) {
        table->buckets[idx] = cur->next;
      } else {
        prev->next = cur->next;
      }
      i_free(table->alloc, cur->key);
      i_free(table->alloc, cur);
      table->size--;
      return 0;
    }
    prev = cur;
    cur = cur->next;
  }
  return -1;
}

size_t hashtable_size(const HashTable *table) {
  return table == NULL ? 0 : table->size;
}

void hashtable_free(HashTable *table) {
  if (table == NULL || table->buckets == NULL) {
    return;
  }
  for (size_t i = 0; i < table->bucket_count; ++i) {
    HashEntry *cur = table->buckets[i];
    while (cur != NULL) {
      HashEntry *next = cur->next;
      i_free(table->alloc, cur->key);
      i_free(table->alloc, cur);
      cur = next;
    }
  }
  i_free(table->alloc, table->buckets);
  table->buckets = NULL;
  table->size = 0;
  table->bucket_count = 0;
}

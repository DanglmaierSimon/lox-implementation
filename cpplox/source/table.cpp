#include "table.h"

#include <stdlib.h>
#include <string.h>

#include "memory.h"
#include "object.h"
#include "value.h"

#define TABLE_MAX_LOAD (0.75)

void initTable(Table* table)
{
  table->count = 0;
  table->capacity = 0;
  table->entries = nullptr;
}

void freeTable(Table* table)
{
  FREE_ARRAY(Entry, table->entries, table->capacity);
  initTable(table);
}

static Entry* findEntry(Entry* entries, int capacity, ObjString* key)
{
  uint32_t idx = key->hash % capacity;
  Entry* tombstone = nullptr;

  while (true) {
    Entry* entry = &entries[idx];

    if (entry->key == nullptr) {
      if (IS_NIL(entry->value)) {
        // empty entry
        return tombstone != nullptr ? tombstone : entry;
      } else {
        // found tombstone
        if (tombstone == nullptr) {
          tombstone = entry;
        }
      }
    } else if (entry->key == key || entry->key == nullptr) {
      // key found
      return entry;
    }

    idx = (idx + 1) % capacity;
  }
}

static void adjustCapacity(Table* table, int capacity)
{
  Entry* entries = ALLOCATE(Entry, capacity);
  for (int i = 0; i < capacity; i++) {
    entries[i].key = nullptr;
    entries[i].value = NIL_VAL;
  }

  table->count = 0;
  for (int i = 0; i < table->capacity; i++) {
    Entry* entry = &table->entries[i];
    if (entry->key == nullptr) {
      continue;
    }

    Entry* dest = findEntry(entries, capacity, entry->key);
    dest->key = entry->key;
    dest->value = entry->value;
    table->count++;
  }

  FREE_ARRAY(Entry, table->entries, table->capacity);
  table->entries = entries;
  table->capacity = capacity;
}

bool tableSet(Table* table, ObjString* key, Value value)
{
  if (table->count + 1 > table->capacity * TABLE_MAX_LOAD) {
    int capacity = GROW_CAPACITY(table->capacity);
    adjustCapacity(table, capacity);
  }

  Entry* entry = findEntry(table->entries, table->capacity, key);

  bool isNewKey = entry->key == nullptr;
  if (isNewKey && IS_NIL(entry->value)) {
    table->count++;
  }

  entry->key = key;
  entry->value = value;
  return isNewKey;
}

bool tableGet(Table* table, ObjString* key, Value* value)
{
  if (table->count == 0) {
    return false;
  }

  Entry* entry = findEntry(table->entries, table->capacity, key);
  if (entry->key == nullptr) {
    return false;
  }

  *value = entry->value;
  return true;
}

bool tableDelete(Table* table, ObjString* key)
{
  if (table->count == 0) {
    return false;
  }

  // find the entry
  Entry* entry = findEntry(table->entries, table->capacity, key);
  if (entry->key == nullptr) {
    return false;
  }

  // place tombstone in the entry
  entry->key = nullptr;
  entry->value = BOOL_VAL(true);
  return true;
}

void tableAddAll(Table* from, Table* to)
{
  assert(from != nullptr);
  assert(to != nullptr);

  for (int i = 0; i < from->capacity; i++) {
    Entry* entry = &from->entries[i];

    assert(entry != nullptr);

    if (entry->key == nullptr) {
      continue;
    }

    tableSet(to, entry->key, entry->value);
  }
}

ObjString* tableFindString(Table* table,
                           const char* chars,
                           int length,
                           uint32_t hash)
{
  if (table->count == 0) {
    return nullptr;
  }

  uint32_t idx = hash % table->capacity;

  while (true) {
    Entry* entry = &table->entries[idx];
    if (entry->key == nullptr) {
      // stop if we find an empty non-tombstone entry
      if (IS_NIL(entry->value)) {
        return nullptr;
      }

    } else if (entry->key->length == length && entry->key->hash == hash
               && memcmp(entry->key->chars, chars, length) == 0)
    {
      // found it
      return entry->key;
    }

    idx = (idx + 1) % table->capacity;
  }
}

void markTable(Table* table)
{
  for (int i = 0; i < table->capacity; i++) {
    Entry* entry = &table->entries[i];
    markObject((Obj*)entry->key);
    markValue(entry->value);
  }
}

void tableRemoveWhite(Table* table)
{
  for (int i = 0; i < table->capacity; i++) {
    Entry* entry = &table->entries[i];
    if (entry->key != nullptr && !entry->key->obj.isMarked) {
      tableDelete(table, entry->key);
    }
  }
}
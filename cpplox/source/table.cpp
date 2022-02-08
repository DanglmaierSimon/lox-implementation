#include <memory>
#include <vector>

#include "lox/table.h"

#include <stdint.h>
#include <string.h>

#include "lox/memory.h"
#include "lox/objects/objstring.h"
#include "lox/value.h"

constexpr auto TABLE_MAX_LOAD = 0.75;

static Entry* findEntry(std::vector<Entry>& entries,
                        int capacity,
                        ObjString* key)
{
  uint32_t idx = key->hash() & (capacity - 1);
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

    idx = (idx + 1) & (capacity - 1);
  }
}

static void adjustCapacity(Table* table, int capacity)
{
  std::vector<Entry> entries;
  entries.resize(capacity);
  for (int i = 0; i < capacity; i++) {
    entries[i].key = nullptr;
    entries[i].value = Value {};
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
  entry->value = Value(true);
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

ObjString* tableFindString(Table* table, std::string string, uint32_t hash)
{
  if (table->count == 0) {
    return nullptr;
  }

  uint32_t idx = hash & (table->capacity - 1);

  while (true) {
    Entry* entry = &table->entries[idx];
    if (entry->key == nullptr) {
      // stop if we find an empty non-tombstone entry
      if (IS_NIL(entry->value)) {
        return nullptr;
      }
    } else if (entry->key->length() == string.length()
               && entry->key->hash() == hash
               && string == entry->key->toString())
    {
      // found it
      return entry->key;
    }

    idx = (idx + 1) & (table->capacity - 1);
  }
}

void markTable(Table* table, MemoryManager* mm)
{
  for (int i = 0; i < table->capacity; i++) {
    Entry* entry = &table->entries[i];
    mm->markObject(entry->key);
    mm->markValue(entry->value);
  }
}

void tableRemoveWhite(Table* table)
{
  for (int i = 0; i < table->capacity; i++) {
    Entry* entry = &table->entries[i];
    if (entry->key != nullptr && !entry->key->isMarked()) {
      tableDelete(table, entry->key);
    }
  }
}
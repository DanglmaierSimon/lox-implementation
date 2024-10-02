#include <cassert>
#include <cstdlib>
#include <cstring>

#include "table.h"

#include "memory.h"
#include "object.h"
#include "value.h"

constexpr auto TABLE_MAX_LOAD = 0.75;

Table::~Table()
{
  FREE_ARRAY(Entry, this->_entries, this->_capacity);
  initTable(this);
}

bool Table::get(ObjString* key, Value* value)
{
  return tableGet(this, key, value);
}

bool Table::set(ObjString* key, Value value)
{
  return tableSet(this, key, value);
}

bool Table::deleteKey(ObjString* key)
{
  return tableDelete(this, key);
}

void Table::addAll(Table* from)
{
  tableAddAll(from, this);
}

ObjString* Table::findString(const char* chars, int length, uint32_t hash)
{
  return tableFindString(this, chars, length, hash);
}

void Table::mark()
{
  markTable(this);
}

void Table::removeWhite()
{
  tableRemoveWhite(this);
}

void Table::initTable(Table* table)
{
  table->_count = 0;
  table->_capacity = 0;
  table->_entries = nullptr;
}

Table::Entry* Table::findEntry(Table::Entry* entries,
                               int capacity,
                               ObjString* key)
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

void Table::adjustCapacity(Table* table, int capacity)
{
  Entry* entries = ALLOCATE(Entry, capacity);
  for (int i = 0; i < capacity; i++) {
    entries[i].key = nullptr;
    entries[i].value = NIL_VAL;
  }

  table->_count = 0;
  for (int i = 0; i < table->_capacity; i++) {
    Entry* entry = &table->_entries[i];
    if (entry->key == nullptr) {
      continue;
    }

    Entry* dest = findEntry(entries, capacity, entry->key);
    dest->key = entry->key;
    dest->value = entry->value;
    table->_count++;
  }

  FREE_ARRAY(Entry, table->_entries, table->_capacity);
  table->_entries = entries;
  table->_capacity = capacity;
}

bool Table::tableSet(Table* table, ObjString* key, Value value)
{
  if (table->_count + 1 > table->_capacity * TABLE_MAX_LOAD) {
    int capacity = GROW_CAPACITY(table->_capacity);
    Table::adjustCapacity(table, capacity);
  }

  Entry* entry = findEntry(table->_entries, table->_capacity, key);

  bool isNewKey = entry->key == nullptr;
  if (isNewKey && IS_NIL(entry->value)) {
    table->_count++;
  }

  entry->key = key;
  entry->value = value;
  return isNewKey;
}

bool Table::tableGet(Table* table, ObjString* key, Value* value)
{
  if (table->_count == 0) {
    return false;
  }

  Entry* entry = findEntry(table->_entries, table->_capacity, key);
  if (entry->key == nullptr) {
    return false;
  }

  *value = entry->value;
  return true;
}

bool Table::tableDelete(Table* table, ObjString* key)
{
  if (table->_count == 0) {
    return false;
  }

  // find the entry
  Entry* entry = findEntry(table->_entries, table->_capacity, key);
  if (entry->key == nullptr) {
    return false;
  }

  // place tombstone in the entry
  entry->key = nullptr;
  entry->value = BOOL_VAL(true);
  return true;
}

void Table::tableAddAll(Table* from, Table* to)
{
  assert(from != nullptr);
  assert(to != nullptr);

  for (int i = 0; i < from->_capacity; i++) {
    Entry* entry = &from->_entries[i];

    assert(entry != nullptr);

    if (entry->key == nullptr) {
      continue;
    }

    tableSet(to, entry->key, entry->value);
  }
}

ObjString* Table::tableFindString(Table* table,
                                  const char* chars,
                                  int length,
                                  uint32_t hash)
{
  if (table->_count == 0) {
    return nullptr;
  }

  uint32_t idx = hash % table->_capacity;

  while (true) {
    Entry* entry = &table->_entries[idx];
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

    idx = (idx + 1) % table->_capacity;
  }
}

void Table::markTable(Table* table)
{
  for (int i = 0; i < table->_capacity; i++) {
    Entry* entry = &table->_entries[i];
    markObject((Obj*)entry->key);
    markValue(entry->value);
  }
}

void Table::tableRemoveWhite(Table* table)
{
  for (int i = 0; i < table->_capacity; i++) {
    Entry* entry = &table->_entries[i];
    if (entry->key != nullptr && !entry->key->obj.isMarked) {
      tableDelete(table, entry->key);
    }
  }
}
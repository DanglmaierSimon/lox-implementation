#include <cassert>
#include <cstdlib>
#include <cstring>

#include "table.h"

#include "memory.h"
#include "object.h"
#include "value.h"

struct Entry
{
  ObjString* key = nullptr;
  Value value;
};

namespace
{
Entry* findEntry(Entry* entries, size_t capacity, ObjString* key)
{
  assert(key != nullptr);

  uint32_t idx = key->hash % capacity;
  Entry* tombstone = nullptr;

  while (true) {
    auto* entry = &entries[idx];

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
}  // namespace

Table::~Table()
{
  FREE_ARRAY<Entry>(this->_entries, this->_capacity);
}

bool Table::isEmpty() const
{
  return _count == 0;
}

bool Table::get(ObjString* key, Value* value) const
{
  if (isEmpty()) {
    return false;
  }

  Entry* entry = findEntry(_entries, _capacity, key);
  if (entry->key == nullptr) {
    return false;
  }

  *value = entry->value;
  return true;
}

bool Table::set(ObjString* key, Value value)
{
  if (static_cast<double>(_count) + 1.0
      > static_cast<double>(_capacity) * TABLE_MAX_LOAD)
  {
    auto capacity = GROW_CAPACITY(_capacity);
    adjustCapacity(capacity);
  }

  auto* entry = findEntry(_entries, _capacity, key);

  bool isNewKey = entry->key == nullptr;
  if (isNewKey && IS_NIL(entry->value)) {
    _count++;
  }

  entry->key = key;
  entry->value = value;
  return isNewKey;
}

bool Table::deleteKey(ObjString* key)
{
  if (isEmpty()) {
    return false;
  }

  // find the entry
  Entry* entry = findEntry(_entries, _capacity, key);
  if (entry->key == nullptr) {
    return false;
  }

  // place tombstone in the entry
  entry->key = nullptr;
  entry->value = BOOL_VAL(true);
  return true;
}

void Table::addAll(Table const& from)
{
  for (size_t i = 0; i < from._capacity; i++) {
    Entry* entry = &from._entries[i];

    assert(entry != nullptr);

    if (entry->key == nullptr) {
      continue;
    }

    set(entry->key, entry->value);
  }
}

ObjString* Table::findString(const char* chars, int length, uint32_t hash) const
{
  if (isEmpty()) {
    return nullptr;
  }

  uint32_t idx = hash % _capacity;

  while (true) {
    Entry* entry = &_entries[idx];
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

    idx = (idx + 1) % _capacity;
  }
}

void Table::mark()
{
  for (size_t i = 0; i < _capacity; i++) {
    auto* entry = &_entries[i];
    markObject((Obj*)entry->key);
    markValue(entry->value);
  }
}

void Table::removeWhite()
{
  for (size_t i = 0; i < _capacity; i++) {
    auto* entry = &_entries[i];
    if (entry->key != nullptr && !entry->key->obj.isMarked) {
      deleteKey(entry->key);
    }
  }
}

void Table::adjustCapacity(size_t capacity)
{
  Entry* entries = ALLOCATE<Entry>(capacity);
  for (size_t i = 0; i < capacity; i++) {
    entries[i].key = nullptr;
    entries[i].value = NIL_VAL;
  }

  _count = 0;
  for (size_t i = 0; i < _capacity; i++) {
    Entry* entry = &_entries[i];
    if (entry->key == nullptr) {
      continue;
    }

    auto* dest = findEntry(entries, capacity, entry->key);
    dest->key = entry->key;
    dest->value = entry->value;
    _count++;
  }

  FREE_ARRAY<Entry>(_entries, _capacity);
  _entries = entries;
  _capacity = capacity;
}

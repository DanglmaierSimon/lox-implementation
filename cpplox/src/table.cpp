#include <cassert>
#include <memory>
#include <optional>
#include <vector>

#include "table.h"

#include "memory.h"
#include "objstring.h"
#include "value.h"

size_t Table::capacity() const
{
  return _capacity;
}

size_t Table::count() const
{
  return _count;
}

std::optional<Value> Table::get(ObjString* key)
{
  if (count() == 0) {
    return std::nullopt;
  }

  Entry* entry = findEntry(_entries.get(), capacity(), key);
  if (entry->key == nullptr) {
    return std::nullopt;
  }

  return std::make_optional(entry->value);
}

Entry* Table::findEntry(Entry* entries, size_t capacity, ObjString* key)
{
  assert(entries != nullptr);
  assert(key != nullptr);

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
    } else if (entry->key == nullptr || entry->key == key) {
      // key found
      return entry;
    }

    idx = (idx + 1) % capacity;
  }
}

void Table::adjustCapacity(size_t newcapacity)
{
  std::unique_ptr<Entry[]> entries = std::make_unique<Entry[]>(newcapacity);

  for (size_t i = 0; i < newcapacity; i++) {
    entries[i].key = nullptr;
    entries[i].value = Value {};
  }

  _count = 0;
  for (size_t i = 0; i < capacity(); i++) {
    Entry* entry = &_entries[i];
    if (entry->key == nullptr) {
      continue;
    }

    Entry* dest = findEntry(entries.get(), newcapacity, entry->key);
    dest->key = entry->key;
    dest->value = entry->value;
    _count++;
  }

  _entries = std::move(entries);
  _capacity = newcapacity;
}

bool Table::set(ObjString* key, Value value)
{
  if ((count() + 1) > (capacity() * TABLE_MAX_LOAD)) {
    size_t newcapacity = GROW_CAPACITY(capacity());
    adjustCapacity(newcapacity);
  }

  Entry* entry = findEntry(_entries.get(), capacity(), key);

  bool isNewKey = entry->key == nullptr;
  if (isNewKey && IS_NIL(entry->value)) {
    _count++;
  }

  entry->key = key;
  entry->value = value;
  return isNewKey;
}

void Table::removeWhite()
{
  for (size_t i = 0; i < capacity(); i++) {
    Entry* entry = &_entries[i];
    if (entry->key != nullptr && !entry->key->isMarked()) {
      remove(entry->key);
    }
  }
}

void Table::mark(MemoryManager* mm)
{
  assert(mm != nullptr);
  for (size_t i = 0; i < capacity(); i++) {
    Entry* entry = &_entries[i];
    mm->markObject(entry->key);
    mm->markValue(entry->value);
  }
}

ObjString* Table::findString(std::string string, uint32_t hash)
{
  if (count() == 0) {
    return nullptr;
  }

  uint32_t idx = hash & (capacity() - 1);

  while (true) {
    Entry* entry = &_entries[idx];
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

    idx = (idx + 1) & (capacity() - 1);
  }
}

void Table::addAll(Table* from)
{
  assert(from != nullptr);

  for (size_t i = 0; i < from->capacity(); i++) {
    Entry* entry = &from->_entries[i];

    assert(entry != nullptr);

    if (entry->key == nullptr) {
      continue;
    }

    set(entry->key, entry->value);
  }
}

bool Table::remove(ObjString* key)
{
  assert(key != nullptr);
  if (count() == 0) {
    return false;
  }

  // find the entry
  Entry* entry = findEntry(_entries.get(), capacity(), key);
  if (entry->key == nullptr) {
    return false;
  }

  // place tombstone in the entry
  entry->key = nullptr;
  entry->value = Value(true);
  return true;
}

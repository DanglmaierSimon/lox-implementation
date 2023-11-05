#pragma once

#include <memory>
#include <optional>
#include <vector>

#include "common.h"
#include "value.h"

class MemoryManager;
class ObjString;

struct Entry
{
  ObjString* key;
  Value value;
};

class Table
{
public:
  size_t count() const;
  size_t capacity() const;

  std::optional<Value> get(ObjString* key);
  bool set(ObjString* key, Value value);

  bool remove(ObjString* key);

  void addAll(Table* from);

  void removeWhite();
  void mark(MemoryManager* mm);

  ObjString* findString(std::string string, uint32_t hash);

private:
  Entry* findEntry(std::vector<Entry>& entries,
                   size_t capacity,
                   ObjString* key);

  void adjustCapacity(size_t newcapacity);

private:
  static constexpr auto TABLE_MAX_LOAD = 0.75;

  size_t _count = 0;
  size_t _capacity = 0;
  std::vector<Entry> _entries;
};

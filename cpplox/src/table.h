#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>

#include "value.h"

struct Entry;

class Table final
{
public:
  Table() = default;
  Table(Table const& other) = delete;

  virtual ~Table();

  std::optional<Value> get(ObjString* key) const;

  bool set(ObjString* key, Value value);

  bool deleteKey(ObjString* key);

  void addAll(const Table& from);

  ObjString* findString(const char* chars, int length, uint32_t hash) const;

  void mark();

  void removeWhite();

  bool isEmpty() const;

private:
  inline static constexpr auto TABLE_MAX_LOAD = 0.75;

  void adjustCapacity(size_t newcapacity);

  size_t _count = 0;
  size_t _capacity = 0;
  Entry* _entries = nullptr;
};

#pragma once

#include <cstdint>

#include "value.h"

class Table final
{
public:
  Table() = default;
  virtual ~Table();

  bool get(ObjString* key, Value* value);

  bool set(ObjString* key, Value value);

  bool deleteKey(ObjString* key);

  void addAll(Table* from);

  ObjString* findString(const char* chars, int length, uint32_t hash);

  void mark();

  void removeWhite();

private:
  struct Entry
  {
    ObjString* key = nullptr;
    Value value;
  };

  static void initTable(Table* table);
  static bool tableGet(Table* table, ObjString* key, Value* value);
  static bool tableSet(Table* table, ObjString* key, Value value);
  static bool tableDelete(Table* table, ObjString* key);
  static void tableAddAll(Table* from, Table* to);
  static ObjString* tableFindString(Table* table,
                                    const char* chars,
                                    int length,
                                    uint32_t hash);
  static void markTable(Table* table);
  static void tableRemoveWhite(Table* table);

  static Entry* findEntry(Entry* entries, int capacity, ObjString* key);
  static void adjustCapacity(Table* table, int capacity);

  int _count = 0;
  int _capacity = 0;
  Entry* _entries = nullptr;
};

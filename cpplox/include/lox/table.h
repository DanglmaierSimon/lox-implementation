#pragma once

#include <memory>
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

struct Table
{
  int count = 0;
  int capacity = 0;
  std::vector<Entry> entries;
};

bool tableGet(Table* table, ObjString* key, Value* value);
bool tableSet(Table* table, ObjString* key, Value value);
bool tableDelete(Table* table, ObjString* key);
void tableAddAll(Table* from, Table* to);
ObjString* tableFindString(Table* table, std::string string, uint32_t hash);

void markTable(Table* table, MemoryManager* mm);
void tableRemoveWhite(Table* table);
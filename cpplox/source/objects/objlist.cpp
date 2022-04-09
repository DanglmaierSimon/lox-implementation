#include "lox/objects/objlist.h"

#include "lox/value.h"

ObjList::ObjList(std::vector<Value> values)
    : _values(std::move(values))
{
}

std::string ObjList::toString() const
{
  std::string ret = "[";

  bool isFirst = true;

  for (const auto& v : values()) {
    if (isFirst) {
      isFirst = false;
    } else {
      ret.append(", ");
    }
    ret.append(::toString(v));
  }

  return ret;
}

ObjType ObjList::type() const
{
  return ObjType::LIST;
}

void ObjList::insert(size_t idx, Value v)
{
  assert(idx < length());
  _values[idx] = v;
}

void ObjList::append(Value v)
{
  _values.push_back(v);
}

void ObjList::append(ObjList other)
{
  for (Value v : other.values()) {
    append(v);
  }
}

Value ObjList::at(size_t idx) const
{
  assert(idx < length());
  return _values[idx];
}

size_t ObjList::length() const
{
  return _values.size();
}

std::vector<Value> ObjList::values() const
{
  return _values;
}
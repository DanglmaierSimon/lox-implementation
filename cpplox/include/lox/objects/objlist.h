#pragma once

#include "lox/objects/obj.h"
#include "lox/value.h"

class ObjList final : public Obj
{
public:
  explicit ObjList(std::vector<Value> values);

  std::string toString() const override;
  ObjType type() const override;

  void insert(size_t idx, Value v);
  void append(Value v);
  void append(ObjList other);

  Value at(size_t idx) const;

  size_t length() const;

  std::vector<Value> values() const;

private:
  std::vector<Value> _values;
};

inline auto IS_LIST(Value value)
{
  return isObjType(value, ObjType::LIST);
}

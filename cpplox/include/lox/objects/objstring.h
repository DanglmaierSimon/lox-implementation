#pragma once

#include <string>

#include "lox/objects/obj.h"
#include "lox/value.h"

class ObjString final : public Obj
{
public:
  ObjString(std::string chars, uint32_t hash);

  uint32_t hash() const;
  size_t length() const;
  std::string string() const;

  std::string toString() const override;
  ObjType type() const override;

private:
  std::string _string;
  uint32_t _hash;
};

inline auto AS_STRING(Value value)
{
  return dynamic_cast<ObjString*>(AS_OBJ(value));
}

inline auto IS_STRING(Value value)
{
  return isObjType(value, ObjType::STRING);
}
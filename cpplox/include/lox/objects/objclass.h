#pragma once

#include "lox/objects/obj.h"
#include "lox/objects/objstring.h"
#include "lox/table.h"

class ObjClass final : public Obj
{
public:
  explicit ObjClass(ObjString* name);

  ObjString* name() const;

  Table* methods();

  std::string toString() const override;

  ObjType type() const override;

private:
  ObjString* _name = nullptr;
  Table _methods;
};

inline auto AS_CLASS(Value value)
{
  return dynamic_cast<ObjClass*>(AS_OBJ(value));
}

inline auto IS_CLASS(Value value)
{
  return isObjType(value, ObjType::CLASS);
}
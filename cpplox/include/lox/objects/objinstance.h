#pragma once

#include "lox/objects/obj.h"
#include "lox/objects/objclass.h"

class ObjInstance final : public Obj
{
public:
  explicit ObjInstance(ObjClass* klass);

  ObjClass* klass() const;

  Table* fields();

  std::string toString() const override;

  ObjType type() const override;

private:
  ObjClass* _klass = nullptr;
  Table _fields;
};

inline auto AS_INSTANCE(Value value)
{
  return dynamic_cast<ObjInstance*>(AS_OBJ(value));
}

inline auto IS_INSTANCE(Value value)
{
  return isObjType(value, ObjType::INSTANCE);
}
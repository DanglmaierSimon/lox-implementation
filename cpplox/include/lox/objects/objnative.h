#pragma once

#include "lox/objects/obj.h"
#include "lox/value.h"

typedef Value (*NativeFn)(int argCount, Value* args);

class ObjNative final : public Obj
{
public:
  explicit ObjNative(NativeFn fn);

  std::string toString() const override;
  ObjType type() const override;

  NativeFn function() const;

private:
  NativeFn _function = nullptr;
};

inline auto AS_NATIVE(Value value)
{
  return dynamic_cast<ObjNative*>(AS_OBJ(value))->function();
}

inline auto IS_NATIVE(Value value)
{
  return isObjType(value, ObjType::NATIVE);
}

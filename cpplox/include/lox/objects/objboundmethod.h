#pragma once

#include "lox/objects/obj.h"
#include "lox/objects/objclosure.h"
#include "lox/value.h"

class ObjBoundMethod final : public Obj
{
public:
  explicit ObjBoundMethod(Value receiver, ObjClosure* method);

  Value receiver() const;

  ObjClosure* method() const;

  std::string toString() const override;
  ObjType type() const override;

private:
  Value _receiver;
  ObjClosure* _method = nullptr;
};

inline auto AS_BOUND_METHOD(Value value)
{
  return dynamic_cast<ObjBoundMethod*>(AS_OBJ(value));
}

inline auto IS_BOUND_METHOD(Value value)
{
  return isObjType(value, ObjType::BOUND_METHOD);
}

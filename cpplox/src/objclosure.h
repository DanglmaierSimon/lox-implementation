#pragma once

#include "obj.h"
#include "objfunction.h"
#include "objupvalue.h"

class ObjClosure final : public Obj
{
public:
  ObjClosure(ObjFunction* function, std::vector<ObjUpvalue*> upvalues);

  ObjFunction* function() const;
  ObjUpvalue* upvalue(int index);

  void setUpvalue(ObjUpvalue* upvalue, int index);

  int upvalueCount() const;

  std::string toString() const override;

  ObjType type() const override;

private:
  ObjFunction* _function = nullptr;
  std::vector<ObjUpvalue*> _upvalues;
};

inline auto AS_CLOSURE(Value value)
{
  return dynamic_cast<ObjClosure*>(AS_OBJ(value));
}

inline auto IS_CLOSURE(Value value)
{
  return isObjType(value, ObjType::CLOSURE);
}

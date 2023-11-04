#pragma once

#include "obj.h"
#include "value.h"

class ObjUpvalue final : public Obj
{
public:
  explicit ObjUpvalue(Value* location, ObjUpvalue* nextUpvalue, Value closed);

  Value* location() const;
  void setLocation(Value* newlocation);

  ObjUpvalue* nextUpvalue() const;
  void setNextUpvalue(ObjUpvalue* next);

  Value* closed();
  void setClosed(Value v);

  std::string toString() const override;
  ObjType type() const override;

private:
  Value* _location = nullptr;  // non-owning, do not delete -> multiple closures
                               // can close over the same variable
  ObjUpvalue* _nextUpvalue = nullptr;
  Value _closed;
};
#include "objclosure.h"

ObjClosure::ObjClosure(ObjFunction* function, std::vector<ObjUpvalue*> upvalues)
    : _function(function)
    , _upvalues(upvalues)
{
}

ObjFunction* ObjClosure::function() const
{
  return _function;
}

std::string ObjClosure::toString() const
{
  return function()->toString();
}

ObjType ObjClosure::type() const
{
  return ObjType::CLOSURE;
}

int ObjClosure::upvalueCount() const
{
  return _upvalues.size();
}

ObjUpvalue* ObjClosure::upvalue(int index)
{
  return _upvalues.at(index);
}

void ObjClosure::setUpvalue(ObjUpvalue* upvalue, int index)
{
  _upvalues[index] = upvalue;
}

#include "lox/objects/objboundmethod.h"

ObjBoundMethod::ObjBoundMethod(Value receiver, ObjClosure* method)
    : _receiver(receiver)
    , _method(method)
{
}

Value ObjBoundMethod::receiver() const
{
  return _receiver;
}

ObjClosure* ObjBoundMethod::method() const
{
  return _method;
}

std::string ObjBoundMethod::toString() const
{
  return method()->function()->toString();
}

ObjType ObjBoundMethod::type() const
{
  return ObjType::BOUND_METHOD;
}

#include <cassert>

#include "objclass.h"

ObjClass::ObjClass(ObjString* name)
    : _name(name)
{
  assert(_name != nullptr);
}

ObjString* ObjClass::name() const
{
  return _name;
}

Table* ObjClass::methods()
{
  return &_methods;
}

std::string ObjClass::toString() const
{
  return name()->string();
}

ObjType ObjClass::type() const
{
  return ObjType::CLASS;
}

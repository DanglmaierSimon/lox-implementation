#include "objinstance.h"

ObjInstance::ObjInstance(ObjClass* klass)
    : _klass(klass)
{
}

std::string ObjInstance::toString() const
{
  return std::string {klass()->name()->string()} + " instance";
}

Table* ObjInstance::fields()
{
  return &_fields;
}

ObjClass* ObjInstance::klass() const
{
  return _klass;
}

ObjType ObjInstance::type() const
{
  return ObjType::INSTANCE;
}

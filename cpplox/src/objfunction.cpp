#include "objfunction.h"

ObjFunction::ObjFunction(int arity, int upvalueCount, ObjString* name)
    : _arity(arity)
    , _upvalueCount(upvalueCount)
    , _name(name)
{
}

int ObjFunction::arity() const
{
  return _arity;
}

int ObjFunction::upvalueCount() const
{
  return _upvalueCount;
}

Chunk* ObjFunction::chunk()
{
  return &_chunk;
}

ObjString* ObjFunction::name() const
{
  return _name;
}

ObjType ObjFunction::type() const
{
  return ObjType::FUNCTION;
}

std::string ObjFunction::toString() const
{
  if (name() == nullptr) {
    return "<script>";
  }
  return std::string {"<fn "} + name()->string() + ">";
}

void ObjFunction::setName(ObjString* name)
{
  _name = name;
}

void ObjFunction::setArity(int arity)
{
  _arity = arity;
}

void ObjFunction::incrementArity()
{
  setArity(arity() + 1);
}

void ObjFunction::setUpvalueCount(int count)
{
  _upvalueCount = count;
}

void ObjFunction::incrementUpvalueCount()
{
  setUpvalueCount(upvalueCount() + 1);
}

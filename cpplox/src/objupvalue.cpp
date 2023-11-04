#include "objupvalue.h"

#include "obj.h"

ObjUpvalue::ObjUpvalue(Value* location, ObjUpvalue* nextUpvalue, Value closed)
    : _location {location}
    , _nextUpvalue {nextUpvalue}
    , _closed(closed)
{
}

ObjType ObjUpvalue::type() const
{
  return ObjType::UPVALUE;
}

std::string ObjUpvalue::toString() const
{
  return "upvalue";
}

Value* ObjUpvalue::location() const
{
  return _location;
}

ObjUpvalue* ObjUpvalue::nextUpvalue() const
{
  return _nextUpvalue;
}

Value* ObjUpvalue::closed()
{
  return &_closed;
}

void ObjUpvalue::setNextUpvalue(ObjUpvalue* next)
{
  _nextUpvalue = next;
}

void ObjUpvalue::setClosed(Value v)
{
  _closed = v;
}

void ObjUpvalue::setLocation(Value* newlocation)
{
  _location = newlocation;
}

#include "obj.h"

void Obj::setNextObj(Obj* next)
{
  _nextObj = next;
}

void Obj::setIsMarked(bool marked)
{
  _isMarked = marked;
}

Obj* Obj::nextObj() const
{
  return _nextObj;
}

bool Obj::isMarked() const
{
  return _isMarked;
}

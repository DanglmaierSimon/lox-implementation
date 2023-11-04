#include "objnative.h"

ObjNative::ObjNative(NativeFn fn)
    : _function(fn)
{
}

NativeFn ObjNative::function() const
{
  return _function;
}

ObjType ObjNative::type() const
{
  return ObjType::NATIVE;
}

std::string ObjNative::toString() const
{
  return "<native fn>";
}

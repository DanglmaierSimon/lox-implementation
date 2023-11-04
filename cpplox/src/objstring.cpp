#include "objstring.h"

ObjString::ObjString(std::string chars, uint32_t hash)
    : _string(chars)
    , _hash {hash}
{
}

ObjType ObjString::type() const
{
  return ObjType::STRING;
}

std::string ObjString::toString() const
{
  return _string;
}

uint32_t ObjString::hash() const
{
  return _hash;
}

std::string ObjString::string() const
{
  return toString();
}

size_t ObjString::length() const
{
  return _string.size();
}
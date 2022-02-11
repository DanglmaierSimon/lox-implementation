#pragma once

#include <memory>

#include "lox/chunk.h"
#include "lox/objects/obj.h"
#include "lox/objects/objstring.h"

class ObjFunction final : public Obj
{
public:
  ObjFunction(int arity, int upvalueCount, ObjString* name = nullptr);

  int arity() const;
  void setArity(int arity);
  void incrementArity();

  int upvalueCount() const;
  void setUpvalueCount(int count);
  void incrementUpvalueCount();

  Chunk* chunk();
  ObjString* name() const;
  void setName(ObjString* name);

  std::string toString() const override;
  ObjType type() const override;

private:
  int _arity = 0;
  int _upvalueCount = 0;

  Chunk _chunk;
  ObjString* _name = nullptr;  // non-owning
};

inline auto AS_FUNCTION(Value value)
{
  return dynamic_cast<ObjFunction*>(AS_OBJ(value));
}

inline auto IS_FUNCTION(Value value)
{
  return isObjType(value, ObjType::FUNCTION);
}
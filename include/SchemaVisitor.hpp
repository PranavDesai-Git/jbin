#pragma once
#include "Schema.hpp"

class SchemaVisitor {
  public:
    virtual ~SchemaVisitor() = default;
    virtual void visit(const Schema &s) = 0;
    virtual void visit(const MessageDef &m) = 0;
    virtual void visit(const EnumDef &e) = 0;
    virtual void visit(const EnumEntry &ee) = 0;
    virtual void visit(const Field &f) = 0;
};

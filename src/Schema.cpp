#include "Schema.hpp"
#include "SchemaVisitor.hpp"

void Schema::accept(SchemaVisitor &visitor) const { visitor.visit(*this); }
void MessageDef::accept(SchemaVisitor &visitor) const { visitor.visit(*this); }
void EnumDef::accept(SchemaVisitor &visitor) const { visitor.visit(*this); }
void EnumEntry::accept(SchemaVisitor &visitor) const { visitor.visit(*this); }
void Field::accept(SchemaVisitor &visitor) const { visitor.visit(*this); }

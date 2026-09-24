#pragma once
#include "SchemaVisitor.hpp"
#include <ostream>
#include <string>

class PythonGenerator : public SchemaVisitor {
  private:
    std::ostream &out;
    std::string currentEnumName;
    const Schema* currentSchema = nullptr;

  public:
    PythonGenerator(std::ostream &outputStream) : out(outputStream) {}

    void visit(const Schema &schema) override;
    void visit(const MessageDef &message) override;
    void visit(const EnumDef &enumDef) override;
    void visit(const EnumEntry &ee) override;
    void visit(const Field &field) override;
};

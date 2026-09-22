#include "SemanticAnalyzer.hpp"
#include "Schema.hpp"
#include <stdexcept>

void SemanticAnalyzer::analyze(const Schema &schema) {
    buildSymbolTable(schema);
    validateMessages(schema);
}

void SemanticAnalyzer::buildSymbolTable(const Schema &schema) {
    for (const auto &e : schema.enums) {

        if (symbolTable.contains(e.name)) {
            throw std::runtime_error("Duplicate enum: " + e.name);
        } else {
            symbolTable.insert(e.name);
        }
    }
    for (const auto &m : schema.messages) {
        if (symbolTable.contains(m.name)) {
            throw std::runtime_error("Duplicate message: " + m.name);
        } else {
            symbolTable.insert(m.name);
        }
    }
}

bool SemanticAnalyzer::isBuiltInType(const std::string &typeName) {
    return typeName == "i32" || typeName == "string" || typeName == "list" ||
           typeName == "map" || typeName == "union";
}


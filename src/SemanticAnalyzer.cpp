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
            throw std::runtime_error("Duplicate enum: " + e.name + " at line " + std::to_string(e.line));
        } else {
            symbolTable.insert(e.name);
        }
    }
    for (const auto &m : schema.messages) {
        if (symbolTable.contains(m.name)) {
            throw std::runtime_error("Duplicate message: " + m.name + " at line " + std::to_string(m.line));
        } else {
            symbolTable.insert(m.name);
        }
    }
}

bool SemanticAnalyzer::isBuiltInType(const std::string &typeName) {
    return typeName == "i32" || typeName == "string" || typeName == "list" ||
           typeName == "map" || typeName == "union";
}

void SemanticAnalyzer::validateMessages(const Schema &schema) {
    for (const MessageDef &m : schema.messages) {
        std::unordered_set<uint32_t> seenTags;

        for (const Field &f : m.fields) {
            if (seenTags.contains(f.number)) {
                throw std::runtime_error("Duplicate Tag number in message at line " + std::to_string(f.line));
            } else {
                seenTags.insert(f.number);
            }

            if (!isBuiltInType(f.type.name) &&
                !symbolTable.contains(f.type.name)) {
                throw std::runtime_error("Unknown type: " + f.type.name + " at line " + std::to_string(f.line));
            }

            if (f.type.name == "map") {
                if (f.type.subTypes.empty()) {
                    throw std::runtime_error("Map requires sub-types (e.g. map(string, i32)) at line " + std::to_string(f.line));
                }
                std::string keyType = f.type.subTypes[0].name;
                if (keyType != "string" && keyType != "i32") {
                    throw std::runtime_error("Map keys must be a scalar type (like 'string' or 'i32') at line " + std::to_string(f.line));
                }
            }
        }
    }
}

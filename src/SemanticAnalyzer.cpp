#include "SemanticAnalyzer.hpp"
#include "Schema.hpp"
#include <stdexcept>
#include <string>

void SemanticAnalyzer::analyze(const Schema &schema) {
    buildSymbolTable(schema);
    validateEnums(schema);
    validateMessages(schema);
    validateCyclicDependencies(schema);
}

void SemanticAnalyzer::validateEnums(const Schema &schema) {
    for (const auto &e : schema.enums) {
        std::unordered_set<uint32_t> seenNumbers;
        for (const auto &entry : e.entries) {
            if (seenNumbers.contains(entry.number)) {
                throw std::runtime_error("Duplicate number in enum " + e.name + " at line " + std::to_string(entry.line));
            }
            seenNumbers.insert(entry.number);
        }
    }
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
    return typeName == "i32" || typeName == "i64" || typeName == "f32" ||
           typeName == "f64" || typeName == "bool" || typeName == "bytes" ||
           typeName == "string" || typeName == "list" || typeName == "map" ||
           typeName == "union";
}

void SemanticAnalyzer::validateMessages(const Schema &schema) {
    for (const MessageDef &m : schema.messages) {
        std::unordered_set<uint32_t> seenTags;

        for (const Field &f : m.fields) {
            // Tag number sanity check for 29-bit limit
            if (f.number > 536870911) {
                throw std::runtime_error("Tag number exceeds maximum allowed (536870911) at line " + std::to_string(f.line));
            }

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
                if (keyType != "string" && keyType != "i32" && keyType != "i64") {
                    throw std::runtime_error("Map keys must be a scalar type (like 'string', 'i32', or 'i64') at line " + std::to_string(f.line));
                }
            }

            // Validate Default Values!
            if (!f.defaultValue.empty()) {
                if (f.type.name == "i32" || f.type.name == "i64") {
                    try {
                        std::stoi(f.defaultValue);
                    } catch (...) {
                        throw std::runtime_error("Invalid default value for integer type at line " + std::to_string(f.line));
                    }
                } else if (f.type.name == "bool") {
                    if (f.defaultValue != "true" && f.defaultValue != "false") {
                        throw std::runtime_error("Invalid default value for bool type at line " + std::to_string(f.line));
                    }
                } else if (!isBuiltInType(f.type.name) && symbolTable.contains(f.type.name)) {
                    // Check if it's an enum, and if so, validate the default value is a valid entry
                    bool isEnum = false;
                    bool isValidEntry = false;
                    for (const auto& e : schema.enums) {
                        if (e.name == f.type.name) {
                            isEnum = true;
                            for (const auto& entry : e.entries) {
                                if (entry.name == f.defaultValue) {
                                    isValidEntry = true;
                                    break;
                                }
                            }
                            break;
                        }
                    }
                    if (isEnum && !isValidEntry) {
                        throw std::runtime_error("Invalid default value '" + f.defaultValue + "' for enum '" + f.type.name + "' at line " + std::to_string(f.line));
                    }
                }
            }
        }
    }
}

bool SemanticAnalyzer::checkCycle(const std::string& currentType, std::unordered_set<std::string>& visited, std::unordered_set<std::string>& recursionStack, const Schema& schema) {
    if (recursionStack.contains(currentType)) return true;
    if (visited.contains(currentType)) return false;

    visited.insert(currentType);
    recursionStack.insert(currentType);

    for (const auto& m : schema.messages) {
        if (m.name == currentType) {
            for (const auto& f : m.fields) {
                // If it's a direct custom type (not in a list, map, or optional), it could be a cycle!
                if (!isBuiltInType(f.type.name) && !f.isOptional) {
                    if (checkCycle(f.type.name, visited, recursionStack, schema)) {
                        return true;
                    }
                }
            }
        }
    }

    recursionStack.erase(currentType);
    return false;
}

void SemanticAnalyzer::validateCyclicDependencies(const Schema &schema) {
    std::unordered_set<std::string> visited;
    std::unordered_set<std::string> recursionStack;
    for (const auto& m : schema.messages) {
        if (checkCycle(m.name, visited, recursionStack, schema)) {
            throw std::runtime_error("Cyclic dependency detected! Message '" + m.name + "' has an infinite size struct.");
        }
    }
}

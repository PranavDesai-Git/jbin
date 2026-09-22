#pragma once
#include "Schema.hpp"
#include <string>
#include <unordered_set>

class SemanticAnalyzer {
  private:
    std::unordered_set<std::string> symbolTable;

    void buildSymbolTable(const Schema &schema);
    void validateEnums(const Schema &schema);
    void validateMessages(const Schema &schema);
    void validateCyclicDependencies(const Schema &schema);
    bool checkCycle(const std::string& currentType, std::unordered_set<std::string>& visited, std::unordered_set<std::string>& recursionStack, const Schema& schema);

    bool isBuiltInType(const std::string &typeName);

  public:
    void analyze(const Schema &schema);
};

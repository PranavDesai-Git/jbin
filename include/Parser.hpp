#pragma once
#include "Lexer.hpp"
#include "Schema.hpp"
#include <vector>
#include <stdexcept>
#include <string>

class Parser {
  private:
    std::vector<Token> tokens;
    size_t curr = 0;

    Token peek() { return tokens[curr]; }
    Token consume() { return tokens[curr++]; }
    bool isAtEnd() {
        return curr >= tokens.size() || peek().type == TokenType::EndOfFile;
    }

    void error(const std::string& msg) {
        int line = curr < tokens.size() ? tokens[curr].line : (tokens.empty() ? 0 : tokens.back().line);
        throw std::runtime_error(msg + " at line " + std::to_string(line));
    }

    DataType parseDataType();
    Field parseField();
    MessageDef parseMessage();
    EnumDef parseEnum();

  public:
    Parser(const std::vector<Token> &tokens) : tokens(tokens) {}

    Schema parse();
};

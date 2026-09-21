#pragma once
#include "Lexer.hpp"
#include "Schema.hpp"
#include <vector>

class Parser {
  private:
    std::vector<Token> tokens;
    size_t curr = 0;

    Token peek() { return tokens[curr]; }
    Token consume() { return tokens[curr++]; }
    bool isAtEnd() {
        return curr >= tokens.size() || peek().type == TokenType::EndOfFile;
    }

    DataType parseDataType();
    Field parseField();
    MessageDef parseMessage();
    EnumDef parseEnum();

  public:
    Parser(const std::vector<Token> &tokens) : tokens(tokens) {}

    Schema parse();
};

#pragma once
#include <string>
#include <vector>

enum class TokenType {
    Keyword_Message,
    Keyword_Enum,
    Keyword_Optional,
    Keyword_Map,
    Keyword_Union,
    Keyword_End,
    Identifier,
    Number,
    StringLiteral,
    Equals,
    Colon,
    Comma,
    Dot,
    LParen,
    RParen,
    EndOfFile
};

struct Token {
    std::string value;
    TokenType type;
};

std::vector<Token> tokenize(const std::string &source);

#include <string>
#include <vector>

enum class TokenType {
    Keyword_Message,
    Keyword_Enum,
    Keyword_End,
    Identifier,
    Number,
    Colon,
    Dot,
    Equals,
    LParen,
    RParen,
    EndOfFile
};

struct Token {
    std::string value;
    TokenType type;
};

std::vector<Token> tokenize(const std::string &source);

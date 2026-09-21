#include "Lexer.hpp"
#include <cctype>
#include <string>
#include <vector>

std::vector<Token> tokenize(const std::string &source) {
    std::vector<Token> result;
    int curr = 0;
    while (curr < source.length()) {
        char c = source[curr];

        if (std::isdigit(c)) {
            std::string num = "";
            while (curr < source.length() && std::isdigit(source[curr])) {
                num += source[curr];
                curr++;
            }
            result.push_back({num, TokenType::Number});
            continue;
        }

        if (c == ':') {
            result.push_back({":", TokenType::Colon});
            curr++;
            continue;
        }
        if (c == '.') {
            result.push_back({".", TokenType::Dot});
            curr++;
            continue;
        }
        if (c == '=') {
            result.push_back({"=", TokenType::Equals});
            curr++;
            continue;
        }
        if (c == '[') {
            result.push_back({"[", TokenType::LParen});
            curr++;
            continue;
        }
        if (c == ']') {
            result.push_back({"]", TokenType::RParen});
            curr++;
            continue;
        }

        if (c == '/' && curr + 1 < source.length() && source[curr + 1] == '/') {
            while (curr < source.length() && source[curr] != '\n') {
                curr++;
            }
            continue;
        }
        if (c == '/' && curr + 1 < source.length() && source[curr + 1] == '*') {
            while (curr + 1 < source.length() &&
                   !(source[curr] == '*' && source[curr + 1] == '/')) {
                curr++;
            }
            curr += 2;
            continue;
        }

        if (std::isalpha(c)) {
            std::string word = "";
            while (curr < source.length() && std::isalpha(source[curr])) {
                word += source[curr];
                curr++;
            }

            if (word == "message") {
                result.push_back({word, TokenType::Keyword_Message});
            } else if (word == "enum") {
                result.push_back({word, TokenType::Keyword_Enum});
            } else if (word == "end") {
                result.push_back({word, TokenType::Keyword_End});
            } else {
                result.push_back({word, TokenType::Identifier});
            }
            continue;
        }

        curr++;
    }
    result.push_back({"", TokenType::EndOfFile});
    return result;
}

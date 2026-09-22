#include "Lexer.hpp"
#include <cctype>
#include <string>
#include <vector>

/*
enum Activity:
    1. active
    2. inactive
end

// comments

message Player:
      1. name: string
      2. health: i32 = 100 // default value
      3. weapons: list(string)
      4. connections: list(Player)
      5. activeStatus: Activity
      6. inventory: map(string, i32)
      7. balance: union(string="empty", i32)
end
*/

std::vector<Token> tokenize(const std::string &source) {
    std::vector<Token> result;
    int curr = 0;
    int currentLine = 1;
    while (curr < source.length()) {
        char c = source[curr];

        if (c == '\n') {
            currentLine++;
            curr++;
            continue;
        }
        
        if (std::isspace(c)) {
            curr++;
            continue;
        }

        if (std::isdigit(c)) {
            std::string num = "";
            while (curr < source.length() && std::isdigit(source[curr])) {
                num += source[curr];
                curr++;
            }
            result.push_back({num, TokenType::Number, currentLine});
            continue;
        }

        if (c == ':') {
            result.push_back({":", TokenType::Colon, currentLine});
            curr++;
            continue;
        }
        if (c == '.') {
            result.push_back({".", TokenType::Dot, currentLine});
            curr++;
            continue;
        }
        if (c == '=') {
            result.push_back({"=", TokenType::Equals, currentLine});
            curr++;
            continue;
        }
        if (c == ',') {
            result.push_back({",", TokenType::Comma, currentLine});
            curr++;
            continue;
        }
        if (c == '(') {
            result.push_back({"(", TokenType::LParen, currentLine});
            curr++;
            continue;
        }
        if (c == ')') {
            result.push_back({")", TokenType::RParen, currentLine});
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
                if (source[curr] == '\n') currentLine++;
                curr++;
            }
            curr += 2;
            continue;
        }
        if (c == '"') {
            std::string text = "";
            curr++;
            while (curr < source.length() && source[curr] != '"') {
                if (source[curr] == '\n') currentLine++;
                text += source[curr];
                curr++;
            }
            curr++;
            result.push_back({text, TokenType::StringLiteral, currentLine});
            continue;
        }

        if (std::isalpha(c)) {
            std::string word = "";
            while (curr < source.length() && std::isalnum(source[curr])) {
                word += source[curr];
                curr++;
            }

            if (word == "message") {
                result.push_back({word, TokenType::Keyword_Message, currentLine});
            } else if (word == "enum") {
                result.push_back({word, TokenType::Keyword_Enum, currentLine});
            } else if (word == "optional") {
                result.push_back({word, TokenType::Keyword_Optional, currentLine});
            } else if (word == "map") {
                result.push_back({word, TokenType::Keyword_Map, currentLine});
            } else if (word == "union") {
                result.push_back({word, TokenType::Keyword_Union, currentLine});
            } else if (word == "end") {
                result.push_back({word, TokenType::Keyword_End, currentLine});
            } else {
                result.push_back({word, TokenType::Identifier, currentLine});
            }
            continue;
        }

        curr++;
    }
    result.push_back({"", TokenType::EndOfFile, currentLine});
    return result;
}

#include "Parser.hpp"
#include "Lexer.hpp"
#include "Schema.hpp"
#include <stdexcept>
#include <string>

DataType Parser::parseDataType() {
    DataType result;

    Token nameToken = consume();
    result.name = nameToken.value;

    if (!isAtEnd() && peek().type == TokenType::LParen) {
        consume();
        result.subTypes.push_back(parseDataType());
        while (!isAtEnd() && peek().type == TokenType::Comma) {
            consume();
            result.subTypes.push_back(parseDataType());
        }
        if (peek().type == TokenType::RParen) {
            consume();
        } else {
            throw std::runtime_error("Expected ')' after type list");
        }
    }

    return result;
}

Field Parser::parseField() {
    Field f;
    f.line = peek().line;
    if (!isAtEnd() && peek().type == TokenType::Keyword_Optional) {
        f.isOptional = true;
        consume();
    }
    if (!isAtEnd() && peek().type == TokenType::Number) {
        int numToken = std::stoi(consume().value);
        f.number = numToken;
    } else {
        throw std::runtime_error(
            "Expected Tag Number at the start of the Field");
    }
    if (!isAtEnd() && peek().type == TokenType::Dot) {
        consume();
    } else {
        throw std::runtime_error("Expected '.' after Number");
    }
    if (!isAtEnd() && peek().type == TokenType::Identifier) {
        std::string nameToken = consume().value;
        f.name = nameToken;
    } else {
        throw std::runtime_error("Expected an identifier");
    }
    if (!isAtEnd() && peek().type == TokenType::Colon) {
        consume();
    } else {
        throw std::runtime_error("Expected ' : ' after an identifier");
    }
    TokenType t = peek().type;
    if (!isAtEnd() &&
        (t == TokenType::Identifier || t == TokenType::Keyword_Map ||
         t == TokenType::Keyword_Union)) {
        DataType typeToken = parseDataType();
        f.type = typeToken;
    } else {
        throw std::runtime_error("Expected a datatype after identifier");
    }
    if (!isAtEnd() && peek().type == TokenType::Equals) {
        consume();
        TokenType t = peek().type;
        if (!isAtEnd() &&
            (t == TokenType::Identifier || t == TokenType::Number ||
             t == TokenType::StringLiteral)) {
            f.defaultValue = consume().value;
        } else {
            throw std::runtime_error("Expected a value after =");
        }
    }
    while (!isAtEnd() && peek().type == TokenType::Comment) {
        f.comment += consume().value + "\n";
    }
    return f;
}

MessageDef Parser::parseMessage() {
    MessageDef m;
    m.line = peek().line;
    if (!isAtEnd() && peek().type == TokenType::Keyword_Message) {
        consume();
    }
    if (!isAtEnd() && peek().type == TokenType::Identifier) {
        std::string nameToken = consume().value;
        m.name = nameToken;
    } else {
        throw std::runtime_error("Expected identifier after message");
    }
    if (!isAtEnd() && peek().type == TokenType::Colon) {
        consume();
    } else {
        throw std::runtime_error("Expected ':' after identifier");
    }
    while (!isAtEnd() && peek().type != TokenType::Keyword_End) {
        if (peek().type == TokenType::Comment) { consume(); continue; }
        m.fields.push_back(parseField());
    }
    consume();
    return m;
}

EnumDef Parser::parseEnum() {
    EnumDef e;
    e.line = peek().line;
    if (!isAtEnd() && peek().type == TokenType::Keyword_Enum) {
        consume();
    }
    if (!isAtEnd() && peek().type == TokenType::Identifier) {
        std::string nameToken = consume().value;
        e.name = nameToken;
    } else {
        throw std::runtime_error("Expected identifier after enum");
    }
    if (!isAtEnd() && peek().type == TokenType::Colon) {
        consume();
    } else {
        throw std::runtime_error("Expected ':' after identifier");
    }
    while (!isAtEnd() && peek().type != TokenType::Keyword_End) {
        if (peek().type == TokenType::Comment) { consume(); continue; }
        EnumEntry ee;
        ee.line = peek().line;
        if (!isAtEnd() && peek().type == TokenType::Number) {
            ee.number = std::stoi(consume().value);
        } else {
            throw std::runtime_error("Expected a number for enum entry");
        }
        if (!isAtEnd() && peek().type == TokenType::Dot) {
            consume();
        } else {
            throw std::runtime_error("Expected '.' after Number");
        }
        if (!isAtEnd() && peek().type == TokenType::Identifier) {
            std::string nameToken = consume().value;
            ee.name = nameToken;
        } else {
            throw std::runtime_error("Expected identifier");
        }
        while (!isAtEnd() && peek().type == TokenType::Comment) {
            ee.comment += consume().value + "\n";
        }
        e.entries.push_back(ee);
    }
    consume();
    return e;
}

Schema Parser::parse() {
    Schema s;
    while (!isAtEnd()) {
        if (peek().type == TokenType::Comment) {
            consume();
            continue;
        }
        if (peek().type == TokenType::Keyword_Package) {
            consume();
            if (peek().type == TokenType::StringLiteral) {
                s.packageName = consume().value;
            } else {
                throw std::runtime_error("Expected string literal after package");
            }
        } else if (peek().type == TokenType::Keyword_Import) {
            consume();
            if (peek().type == TokenType::StringLiteral) {
                s.imports.push_back(consume().value);
            } else {
                throw std::runtime_error("Expected string literal after import");
            }
        } else if (peek().type == TokenType::Keyword_Message)
            s.messages.push_back(parseMessage());
        else if (peek().type == TokenType::Keyword_Enum)
            s.enums.push_back(parseEnum());
        else
            throw std::runtime_error("Unexpected token in global scope");
    }
    return s;
}

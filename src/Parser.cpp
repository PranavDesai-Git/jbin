#include "Parser.hpp"
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

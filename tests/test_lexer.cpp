#include <catch2/catch_test_macros.hpp>
#include "Lexer.hpp"

TEST_CASE("Lexer identifies basic tokens", "[lexer]") {
    auto tokens = tokenize("message User:\n 1. id: i32 = 10 \nend");
    REQUIRE(tokens.size() == 12);
    REQUIRE(tokens[0].type == TokenType::Keyword_Message);
    REQUIRE(tokens[1].type == TokenType::Identifier);
    REQUIRE(tokens[1].value == "User");
    REQUIRE(tokens[2].type == TokenType::Colon);
    REQUIRE(tokens[3].type == TokenType::Number);
    REQUIRE(tokens[4].type == TokenType::Dot);
    REQUIRE(tokens[5].type == TokenType::Identifier);
    REQUIRE(tokens[6].type == TokenType::Colon);
    REQUIRE(tokens[7].type == TokenType::Identifier);
    REQUIRE(tokens[8].type == TokenType::Equals);
    REQUIRE(tokens[9].type == TokenType::Number);
    REQUIRE(tokens[9].value == "10");
    REQUIRE(tokens[10].type == TokenType::Keyword_End);
    REQUIRE(tokens[11].type == TokenType::EndOfFile);
}

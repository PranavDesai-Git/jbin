#include <catch2/catch_test_macros.hpp>
#include "Lexer.hpp"
#include "Parser.hpp"

TEST_CASE("Parser creates schema from tokens", "[parser]") {
    Parser parser(tokenize("message User:\n 1. id: i32\nend"));
    Schema schema = parser.parse();
    
    REQUIRE(schema.messages.size() == 1);
    REQUIRE(schema.messages[0].name == "User");
    REQUIRE(schema.messages[0].fields.size() == 1);
    REQUIRE(schema.messages[0].fields[0].name == "id");
    REQUIRE(schema.messages[0].fields[0].type.name == "i32");
}

TEST_CASE("Parser includes line numbers in errors", "[parser]") {
    Parser parser(tokenize("message User:\n 1. id i32\nend")); // missing colon
    
    try {
        parser.parse();
        FAIL("Expected an exception");
    } catch (const std::runtime_error& e) {
        REQUIRE(std::string(e.what()).find("at line 2") != std::string::npos);
    }
}

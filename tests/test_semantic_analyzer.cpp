#include <catch2/catch_test_macros.hpp>
#include "Lexer.hpp"
#include "Parser.hpp"
#include "SemanticAnalyzer.hpp"

TEST_CASE("SemanticAnalyzer detects cyclic dependencies", "[semantic]") {
    Parser parser(tokenize("message A:\n 1. b: B\nend\nmessage B:\n 1. a: A\nend"));
    Schema schema = parser.parse();
    
    SemanticAnalyzer analyzer;
    try {
        analyzer.analyze(schema);
        FAIL("Expected cyclic dependency exception");
    } catch (const std::runtime_error& e) {
        REQUIRE(std::string(e.what()).find("Cyclic dependency") != std::string::npos);
    }
}

TEST_CASE("SemanticAnalyzer accepts valid schemas", "[semantic]") {
    Parser parser(tokenize("message A:\n 1. id: i32\nend\nmessage B:\n 1. a: A\nend"));
    Schema schema = parser.parse();
    
    SemanticAnalyzer analyzer;
    REQUIRE_NOTHROW(analyzer.analyze(schema));
}

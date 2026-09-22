#include "Lexer.hpp"
#include "Parser.hpp"
#include "SemanticAnalyzer.hpp"
#include <iostream>

int main() {
    // I am intentionally writing a broken schema to test the Semantic Analyzer!
    // Can you spot the two errors?
    std::string schemaText = R"(
        // This is a test schema!
        package "com.game.core"
        import "math.jbin"

        enum Activity:
            1. active
            2. inactive // the user went offline
        end

        message Player:
            1. name: string
            2. health: i32 = 100
            3. activeStatus: Activity
        end
    )";

    try {
        std::vector<Token> tokens = tokenize(schemaText);

        Parser parser(tokens);
        Schema schema = parser.parse();

        SemanticAnalyzer analyzer;
        analyzer.analyze(schema);

        std::cout << "Successfully parsed schema" << std::endl;
        std::cout << "Package: " << schema.packageName << std::endl;
        std::cout << "Imports: ";
        for (const auto& imp : schema.imports) std::cout << imp << " ";
        std::cout << "\nFound " << schema.messages.size() << " messages and "
                  << schema.enums.size() << " enums." << std::endl;

    } catch (const std::exception &e) {
        std::cerr << "Compiler Error: " << e.what() << std::endl;
    }

    return 0;
}

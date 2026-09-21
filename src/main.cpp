#include "Lexer.hpp"
#include "Parser.hpp"
#include <iostream>

int main() {
    std::string schemaText = R"(
            enum Activity:
                1. active
                2. inactive
            end

            message Player:
                1. name: string
            end
        )";

    try {
        std::vector<Token> tokens = tokenize(schemaText);

        Parser parser(tokens);
        Schema schema = parser.parse();

        std::cout << "Successfully parsed schema" << std::endl;
        std::cout << "Found " << schema.messages.size() << " messages and "
                  << schema.enums.size() << " enums." << std::endl;

    } catch (const std::exception &e) {
        std::cerr << "Compiler Error: " << e.what() << std::endl;
    }

    return 0;
}

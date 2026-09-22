#include "DynamicPacker.hpp"
#include "Lexer.hpp"
#include "Parser.hpp"
#include "SemanticAnalyzer.hpp"
#include <cxxopts.hpp>
#include <fstream>
#include <iostream>
#include <sstream>

int main(int argc, char *argv[]) {
    try {
        cxxopts::Options options("jbin", "jbin schema compiler");

        options.add_options()
            ("command", "Command to run (e.g. build)", cxxopts::value<std::string>())
            ("input", "Input schema file", cxxopts::value<std::string>())
            ("o,out", "Output language (c, cpp, python, js)", cxxopts::value<std::string>())
            ("h,help", "Print usage");

        options.parse_positional({"command", "input"});
        auto result = options.parse(argc, argv);

        if (result.count("help") || result.arguments().empty()) {
            std::cout << options.help() << std::endl;
            return 0;
        }

        std::string command = result["command"].as<std::string>();
        if (command == "build") {
            if (!result.count("input")) {
                std::cerr << "Error: No input file specified." << std::endl;
                return 1;
            }
            if (!result.count("out")) {
                std::cerr << "Error: --out flag is required (e.g., --out c)." << std::endl;
                return 1;
            }

            std::string inputFile = result["input"].as<std::string>();
            std::string targetLang = result["out"].as<std::string>();

            std::ifstream file(inputFile);
            if (!file.is_open()) {
                std::cerr << "Error: Could not open file " << inputFile << std::endl;
                return 1;
            }

            std::stringstream buffer;
            buffer << file.rdbuf();
            std::string schemaText = buffer.str();

            std::cout << "Compiling " << inputFile << " for target " << targetLang << "..." << std::endl;

            std::vector<Token> tokens = tokenize(schemaText);
            Parser parser(tokens);
            Schema schema = parser.parse();

            SemanticAnalyzer analyzer;
            analyzer.analyze(schema);

            std::cout << "Successfully parsed schema!" << std::endl;
            std::cout << "Package: " << schema.packageName << std::endl;
            std::cout << "Found " << schema.messages.size() << " messages and "
                      << schema.enums.size() << " enums." << std::endl;

            // TODO: Pass schema to Codegen Visitor based on targetLang!
        } else {
            std::cerr << "Unknown command: " << command << std::endl;
            return 1;
        }
    } catch (const std::exception &e) {
        std::cerr << "Compiler Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}

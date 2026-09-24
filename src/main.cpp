#include "CGenerator.hpp"
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

        options.add_options()("command", "Command to run (e.g. build, pack)",
                              cxxopts::value<std::string>())(
            "input", "Input schema file", cxxopts::value<std::string>())(
            "o,out",
            "Output (target language for build, or output binary file for "
            "pack)",
            cxxopts::value<std::string>())("j,json",
                                           "Input JSON file (for pack command)",
                                           cxxopts::value<std::string>())(
            "m,msg", "Root message name to pack (for pack command)",
            cxxopts::value<std::string>())("h,help", "Print usage");

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
                std::cerr << "Error: --out flag is required (e.g., --out c)."
                          << std::endl;
                return 1;
            }

            std::string inputFile = result["input"].as<std::string>();
            std::string targetLang = result["out"].as<std::string>();

            std::ifstream file(inputFile);
            if (!file.is_open()) {
                std::cerr << "Error: Could not open file " << inputFile
                          << std::endl;
                return 1;
            }

            std::stringstream buffer;
            buffer << file.rdbuf();
            std::string schemaText = buffer.str();

            std::vector<Token> tokens = tokenize(schemaText);
            Parser parser(tokens);
            Schema schema = parser.parse();

            SemanticAnalyzer analyzer;
            analyzer.analyze(schema);

            if (targetLang == "c") {
                CGenerator cGen(std::cout);
                schema.accept(cGen);
            } else {
                std::cerr << "Code generation for '" << targetLang
                          << "' is not supported yet!" << std::endl;
            }
        } else if (command == "pack") {
            if (!result.count("input") || !result.count("json") ||
                !result.count("msg") || !result.count("out")) {
                std::cerr
                    << "Error: pack requires schema, --json, --msg, and --out"
                    << std::endl;
                return 1;
            }
            std::string schemaFile = result["input"].as<std::string>();
            std::string jsonFile = result["json"].as<std::string>();
            std::string msgName = result["msg"].as<std::string>();
            std::string outFile = result["out"].as<std::string>();

            // Parse schema
            std::ifstream sfile(schemaFile);
            std::stringstream sbuffer;
            sbuffer << sfile.rdbuf();
            Parser parser(tokenize(sbuffer.str()));
            Schema schema = parser.parse();
            SemanticAnalyzer().analyze(schema);

            // Read JSON
            std::ifstream jfile(jsonFile);
            if (!jfile.is_open()) {
                std::cerr << "Error: Could not open JSON file " << jsonFile
                          << std::endl;
                return 1;
            }
            nlohmann::json jsonData;
            jfile >> jsonData;

            // Pack
            DynamicPacker packer(schema);
            std::vector<uint8_t> bin = packer.pack(msgName, jsonData);

            // Write Bin
            std::ofstream bfile(outFile, std::ios::binary);
            bfile.write(reinterpret_cast<const char *>(bin.data()), bin.size());
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

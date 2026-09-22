#include "DynamicPacker.hpp"
#include "Lexer.hpp"
#include "Parser.hpp"
#include "SemanticAnalyzer.hpp"
#include <iomanip>
#include <iostream>

int main() {
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
            4. weapons: list(string)
            5. connections: list(Player)
            6. inventory: map(string, i32)
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
        for (const auto &imp : schema.imports)
            std::cout << imp << " ";
        std::cout << "\nFound " << schema.messages.size() << " messages and "
                  << schema.enums.size() << " enums." << std::endl;

        nlohmann::json testJson = {
            {"name", "Hero"},
            {"health", 100},
            {"activeStatus", "active"},
            {"weapons", {"Sword", "Shield"}},
            {"connections", {
                {{"name", "NPC1"}, {"health", 50}}
            }},
            {"inventory", {
                {"gold", 500},
                {"potions", 3}
            }}
        }; DynamicPacker packer(schema);
        std::vector<uint8_t> binaryBuffer = packer.pack("Player", testJson);

        std::cout << "\nPacked into " << binaryBuffer.size() << " bytes: ";
        for (uint8_t b : binaryBuffer) {
            std::cout << std::hex << std::setw(2) << std::setfill('0') << (int)b
                      << " ";
        }
        std::cout << std::dec << "\n";

        DynamicReader reader(schema);
        nlohmann::json unpackedJson = reader.unpack("Player", binaryBuffer);

        std::cout << "Unpacked JSON: \n" << unpackedJson.dump(4) << "\n";

    } catch (const std::exception &e) {
        std::cerr << "Compiler Error: " << e.what() << std::endl;
    }

    return 0;
}

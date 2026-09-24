#include <catch2/catch_test_macros.hpp>
#include "DynamicPacker.hpp"
#include "Schema.hpp"

TEST_CASE("DynamicPacker packs and unpacks built-in types", "[packer]") {
    Schema schema;
    MessageDef msg;
    msg.name = "TestMessage";
    
    // Setup fields for all supported primitive types
    msg.fields.push_back({1, "test_i32", {"i32", {}}});
    msg.fields.push_back({2, "test_i64", {"i64", {}}});
    msg.fields.push_back({3, "test_f32", {"f32", {}}});
    msg.fields.push_back({4, "test_f64", {"f64", {}}});
    msg.fields.push_back({5, "test_string", {"string", {}}});
    msg.fields.push_back({6, "test_bool", {"bool", {}}});
    msg.fields.push_back({7, "test_bytes", {"bytes", {}}});
    
    schema.messages.push_back(msg);
    
    DynamicPacker packer(schema);
    DynamicReader reader(schema);
    
    nlohmann::json original = nlohmann::json::object();
    original["test_i32"] = -12345;
    original["test_i64"] = 9876543210LL;
    original["test_f32"] = 3.14159f;
    original["test_f64"] = 2.718281828459;
    original["test_string"] = "Hello, jbin!";
    original["test_bool"] = true;
    
    // Bytes as an array of ints
    std::vector<uint8_t> myBytes = {0xDE, 0xAD, 0xBE, 0xEF};
    original["test_bytes"] = myBytes;
    
    std::vector<uint8_t> buffer = packer.pack("TestMessage", original);
    
    REQUIRE(buffer.size() > 0);
    
    nlohmann::json unpacked = reader.unpack("TestMessage", buffer);
    
    REQUIRE(unpacked["test_i32"].get<int32_t>() == -12345);
    REQUIRE(unpacked["test_i64"].get<int64_t>() == 9876543210LL);
    
    // Float comparison with epsilon
    REQUIRE(std::abs(unpacked["test_f32"].get<float>() - 3.14159f) < 0.0001f);
    REQUIRE(std::abs(unpacked["test_f64"].get<double>() - 2.718281828459) < 0.0001);
    
    REQUIRE(unpacked["test_string"].get<std::string>() == "Hello, jbin!");
    REQUIRE(unpacked["test_bool"].get<bool>() == true);
    
    std::vector<uint8_t> unpackedBytes = unpacked["test_bytes"].get<std::vector<uint8_t>>();
    REQUIRE(unpackedBytes == myBytes);
}

TEST_CASE("DynamicPacker packs and unpacks union", "[packer]") {
    Schema schema;
    MessageDef msg;
    msg.name = "TestUnion";
    
    // field 1 is union(string, i32)
    DataType stringType{"string", {}};
    DataType i32Type{"i32", {}};
    msg.fields.push_back({1, "val", {"union", {stringType, i32Type}}});
    
    schema.messages.push_back(msg);
    
    DynamicPacker packer(schema);
    DynamicReader reader(schema);
    
    // Test packing the i32 variant
    nlohmann::json json1 = nlohmann::json::object();
    json1["val"] = 42;
    std::vector<uint8_t> buf1 = packer.pack("TestUnion", json1);
    nlohmann::json out1 = reader.unpack("TestUnion", buf1);
    REQUIRE(out1["val"].get<int>() == 42);
    
    // Test packing the string variant
    nlohmann::json json2 = nlohmann::json::object();
    json2["val"] = "success";
    std::vector<uint8_t> buf2 = packer.pack("TestUnion", json2);
    nlohmann::json out2 = reader.unpack("TestUnion", buf2);
    REQUIRE(out2["val"].get<std::string>() == "success");
}

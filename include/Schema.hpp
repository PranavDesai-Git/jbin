#pragma once
#include <cstdint>
#include <string>
#include <vector>

struct DataType {
    std::string name;
    std::vector<DataType> subTypes;
};

struct Field {
    uint32_t number;
    std::string name;
    DataType type;
    bool isOptional = false;
    std::string defaultValue = "";
};

struct MessageDef {
    std::string name;
    std::vector<Field> fields;
};

struct EnumEntry {
    uint32_t number;
    std::string name;
};

struct EnumDef {
    std::string name;
    std::vector<EnumEntry> entries;
};

struct Schema {
    std::vector<EnumDef> enums;
    std::vector<MessageDef> messages;
};

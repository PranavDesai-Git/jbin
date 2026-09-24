#pragma once
#include <cstdint>
#include <string>
#include <vector>

class SchemaVisitor;

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
    int line = 0;
    std::string comment = "";

    void accept(SchemaVisitor &visitor) const;
};

struct MessageDef {
    std::string name;
    std::vector<Field> fields;
    int line = 0;
    std::string comment = "";

    void accept(SchemaVisitor &visitor) const;
};

struct EnumEntry {
    uint32_t number;
    std::string name;
    int line = 0;
    std::string comment = "";

    void accept(SchemaVisitor &visitor) const;
};

struct EnumDef {
    std::string name;
    std::vector<EnumEntry> entries;
    int line = 0;
    std::string comment = "";

    void accept(SchemaVisitor &visitor) const;
};

struct Schema {
    std::string packageName = "";
    std::vector<std::string> imports;
    std::vector<EnumDef> enums;
    std::vector<MessageDef> messages;

    void accept(SchemaVisitor &visitor) const;
};

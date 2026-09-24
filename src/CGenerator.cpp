#include "CGenerator.hpp"
#include "Schema.hpp"
#include <iostream>
#include <string>
#include <set>

// Helper to determine wire type
bool isEnum(const std::string& name, const Schema* schema) {
    if (!schema) return false;
    for (const auto& e : schema->enums) {
        if (e.name == name) return true;
    }
    return false;
}

std::string getCType(const DataType& type, const Schema* schema) {
    if (type.name == "i32") return "int32_t";
    if (type.name == "i64") return "int64_t";
    if (type.name == "string") return "char*";
    if (type.name == "bool") return "bool";
    if (type.name == "list") {
        return "jbin_list_" + getCType(type.subTypes[0], schema);
    }
    if (type.name == "map") {
        return "jbin_map_" + getCType(type.subTypes[0], schema) + "_" + getCType(type.subTypes[1], schema);
    }
    return type.name;
}

std::string getCName(const DataType& type) {
    if (type.name == "list") return "list_" + getCName(type.subTypes[0]);
    if (type.name == "map") return "map_" + getCName(type.subTypes[0]) + "_" + getCName(type.subTypes[1]);
    if (type.name == "string") return "char_ptr";
    if (type.name == "i32") return "int32";
    if (type.name == "i64") return "int64";
    return type.name;
}

void CGenerator::visit(const Schema &schema) {
    currentSchema = &schema;

    out << "#pragma once\n";
    out << "#include <stdint.h>\n";
    out << "#include <stdbool.h>\n";
    out << "#include <string.h>\n";
    out << "#include <stdlib.h>\n\n";

    // Common C Runtime
    out << "static inline void jbin_encode_varint(uint8_t* buffer, size_t* offset, uint64_t value) {\n"
        << "    while (value >= 0x80) {\n"
        << "        buffer[(*offset)++] = (value & 0x7F) | 0x80;\n"
        << "        value >>= 7;\n"
        << "    }\n"
        << "    buffer[(*offset)++] = (uint8_t)(value & 0x7F);\n"
        << "}\n\n";

    out << "static inline uint64_t jbin_decode_varint(const uint8_t* buffer, size_t* offset) {\n"
        << "    uint64_t result = 0;\n"
        << "    int shift = 0;\n"
        << "    while (1) {\n"
        << "        uint8_t byte = buffer[(*offset)++];\n"
        << "        result |= (uint64_t)(byte & 0x7F) << shift;\n"
        << "        if (!(byte & 0x80)) break;\n"
        << "        shift += 7;\n"
        << "    }\n"
        << "    return result;\n"
        << "}\n\n";

    out << "static inline void jbin_encode_string(uint8_t* buffer, size_t* offset, const char* str) {\n"
        << "    size_t len = strlen(str);\n"
        << "    jbin_encode_varint(buffer, offset, len);\n"
        << "    memcpy(buffer + (*offset), str, len);\n"
        << "    *offset += len;\n"
        << "}\n\n";

    out << "static inline char* jbin_decode_string(const uint8_t* buffer, size_t* offset) {\n"
        << "    size_t len = jbin_decode_varint(buffer, offset);\n"
        << "    char* str = (char*)malloc(len + 1);\n"
        << "    memcpy(str, buffer + (*offset), len);\n"
        << "    str[len] = '\\0';\n"
        << "    *offset += len;\n"
        << "    return str;\n"
        << "}\n\n";

    // Forward declare structs
    for (const auto &m : schema.messages) {
        out << "typedef struct " << m.name << " " << m.name << ";\n";
    }
    out << "\n";

    // Lists and Maps structs
    std::set<std::string> generated;
    for (const auto &m : schema.messages) {
        for (const auto &f : m.fields) {
            if (f.type.name == "list" || f.type.name == "map") {
                std::string cname = "jbin_" + getCName(f.type);
                if (generated.find(cname) == generated.end()) {
                    generated.insert(cname);
                    if (f.type.name == "list") {
                        out << "typedef struct {\n"
                            << "    " << getCType(f.type.subTypes[0], currentSchema) << "* data;\n"
                            << "    size_t length;\n"
                            << "    size_t capacity;\n"
                            << "} " << cname << ";\n\n";
                    } else if (f.type.name == "map") {
                        out << "typedef struct {\n"
                            << "    " << getCType(f.type.subTypes[0], currentSchema) << "* keys;\n"
                            << "    " << getCType(f.type.subTypes[1], currentSchema) << "* values;\n"
                            << "    size_t length;\n"
                            << "    size_t capacity;\n"
                            << "} " << cname << ";\n\n";
                    }
                }
            }
        }
    }

    for (const auto &e : schema.enums) {
        e.accept(*this);
    }
    for (const auto &m : schema.messages) {
        m.accept(*this);
    }
}

void CGenerator::visit(const EnumDef &enumDef) {
    currentEnumName = enumDef.name;
    out << "typedef enum {\n";
    for (const auto &ee : enumDef.entries) {
        ee.accept(*this);
    }
    out << "} " << enumDef.name << ";\n\n";
}

void CGenerator::visit(const EnumEntry &ee) {
    out << "    " << currentEnumName << "_" << ee.name << " = " << ee.number << ",\n";
}

void CGenerator::visit(const MessageDef &message) {
    out << "struct " << message.name << " {\n";
    for (const auto &field : message.fields) {
        field.accept(*this);
    }
    out << "};\n\n";

    // GENERATE PACK
    out << "size_t " << message.name << "_pack(const " << message.name << "* msg, uint8_t* buffer) {\n";
    out << "    size_t offset = 0;\n";
    for (const auto &f : message.fields) {
        bool varint = (f.type.name == "i32" || f.type.name == "i64" || f.type.name == "bool" || isEnum(f.type.name, currentSchema));
        uint8_t wiretype = varint ? 0 : 3;

        out << "    // Field " << f.name << "\n";
        
        if (varint) {
            out << "    jbin_encode_varint(buffer, &offset, (" << f.number << " << 3) | 0);\n";
            out << "    jbin_encode_varint(buffer, &offset, (uint64_t)msg->" << f.name << ");\n";
        } else if (f.type.name == "string") {
            out << "    if (msg->" << f.name << ") {\n";
            out << "        jbin_encode_varint(buffer, &offset, (" << f.number << " << 3) | 3);\n";
            out << "        jbin_encode_string(buffer, &offset, msg->" << f.name << ");\n";
            out << "    }\n";
        } else if (f.type.name == "list") {
            bool subVarint = (f.type.subTypes[0].name == "i32" || f.type.subTypes[0].name == "i64" || f.type.subTypes[0].name == "bool" || isEnum(f.type.subTypes[0].name, currentSchema));
            out << "    for (size_t i = 0; i < msg->" << f.name << ".length; i++) {\n";
            if (subVarint) {
                out << "        jbin_encode_varint(buffer, &offset, (" << f.number << " << 3) | 0);\n";
                out << "        jbin_encode_varint(buffer, &offset, (uint64_t)msg->" << f.name << ".data[i]);\n";
            } else if (f.type.subTypes[0].name == "string") {
                out << "        jbin_encode_varint(buffer, &offset, (" << f.number << " << 3) | 3);\n";
                out << "        jbin_encode_string(buffer, &offset, msg->" << f.name << ".data[i]);\n";
            } else {
                out << "        uint8_t* temp = (uint8_t*)malloc(4096);\n";
                out << "        size_t temp_len = " << f.type.subTypes[0].name << "_pack(&msg->" << f.name << ".data[i], temp);\n";
                out << "        jbin_encode_varint(buffer, &offset, (" << f.number << " << 3) | 3);\n";
                out << "        jbin_encode_varint(buffer, &offset, temp_len);\n";
                out << "        memcpy(buffer + offset, temp, temp_len);\n";
                out << "        offset += temp_len;\n";
                out << "        free(temp);\n";
            }
            out << "    }\n";
        } else if (f.type.name == "map") {
            out << "    for (size_t i = 0; i < msg->" << f.name << ".length; i++) {\n";
            out << "        uint8_t* temp = (uint8_t*)malloc(4096);\n";
            out << "        size_t temp_off = 0;\n";
            
            bool kVarint = (f.type.subTypes[0].name == "i32" || f.type.subTypes[0].name == "i64");
            if (kVarint) {
                out << "        jbin_encode_varint(temp, &temp_off, (1 << 3) | 0);\n";
                out << "        jbin_encode_varint(temp, &temp_off, (uint64_t)msg->" << f.name << ".keys[i]);\n";
            } else {
                out << "        jbin_encode_varint(temp, &temp_off, (1 << 3) | 3);\n";
                out << "        jbin_encode_string(temp, &temp_off, msg->" << f.name << ".keys[i]);\n";
            }

            bool vVarint = (f.type.subTypes[1].name == "i32" || f.type.subTypes[1].name == "i64" || f.type.subTypes[1].name == "bool" || isEnum(f.type.subTypes[1].name, currentSchema));
            if (vVarint) {
                out << "        jbin_encode_varint(temp, &temp_off, (2 << 3) | 0);\n";
                out << "        jbin_encode_varint(temp, &temp_off, (uint64_t)msg->" << f.name << ".values[i]);\n";
            } else if (f.type.subTypes[1].name == "string") {
                out << "        jbin_encode_varint(temp, &temp_off, (2 << 3) | 3);\n";
                out << "        jbin_encode_string(temp, &temp_off, msg->" << f.name << ".values[i]);\n";
            } else {
                out << "        uint8_t* vtemp = (uint8_t*)malloc(4096);\n";
                out << "        size_t vtemp_len = " << f.type.subTypes[1].name << "_pack(&msg->" << f.name << ".values[i], vtemp);\n";
                out << "        jbin_encode_varint(temp, &temp_off, (2 << 3) | 3);\n";
                out << "        jbin_encode_varint(temp, &temp_off, vtemp_len);\n";
                out << "        memcpy(temp + temp_off, vtemp, vtemp_len);\n";
                out << "        temp_off += vtemp_len;\n";
                out << "        free(vtemp);\n";
            }

            out << "        jbin_encode_varint(buffer, &offset, (" << f.number << " << 3) | 3);\n";
            out << "        jbin_encode_varint(buffer, &offset, temp_off);\n";
            out << "        memcpy(buffer + offset, temp, temp_off);\n";
            out << "        offset += temp_off;\n";
            out << "        free(temp);\n";
            out << "    }\n";
        } else {
            // Nested Message
            out << "    {\n";
            out << "        uint8_t* temp = (uint8_t*)malloc(4096);\n";
            out << "        size_t temp_len = " << f.type.name << "_pack(&msg->" << f.name << ", temp);\n";
            out << "        jbin_encode_varint(buffer, &offset, (" << f.number << " << 3) | 3);\n";
            out << "        jbin_encode_varint(buffer, &offset, temp_len);\n";
            out << "        memcpy(buffer + offset, temp, temp_len);\n";
            out << "        offset += temp_len;\n";
            out << "        free(temp);\n";
            out << "    }\n";
        }
    }
    out << "    return offset;\n}\n\n";

    // GENERATE UNPACK
    out << "bool " << message.name << "_unpack(const uint8_t *buffer, size_t length, " << message.name << "* out_msg) {\n";
    out << "    size_t offset = 0;\n";
    out << "    while(offset < length) {\n";
    out << "        uint32_t tag = jbin_decode_varint(buffer, &offset);\n";
    out << "        uint32_t field_num = tag >> 3;\n";
    out << "        switch(field_num) {\n";
    for (const auto& f : message.fields) {
        bool varint = (f.type.name == "i32" || f.type.name == "i64" || f.type.name == "bool" || isEnum(f.type.name, currentSchema));
        
        out << "            case " << f.number << ":\n";
        if (varint) {
            out << "                out_msg->" << f.name << " = jbin_decode_varint(buffer, &offset);\n";
        } else if (f.type.name == "string") {
            out << "                out_msg->" << f.name << " = jbin_decode_string(buffer, &offset);\n";
        } else if (f.type.name == "list") {
            std::string subC = getCType(f.type.subTypes[0], currentSchema);
            bool subVarint = (f.type.subTypes[0].name == "i32" || f.type.subTypes[0].name == "i64" || f.type.subTypes[0].name == "bool" || isEnum(f.type.subTypes[0].name, currentSchema));
            out << "                {\n";
            out << "                    out_msg->" << f.name << ".length++;\n";
            out << "                    out_msg->" << f.name << ".data = (" << subC << "*)realloc(out_msg->" << f.name << ".data, out_msg->" << f.name << ".length * sizeof(" << subC << "));\n";
            out << "                    size_t idx = out_msg->" << f.name << ".length - 1;\n";
            if (subVarint) {
                out << "                    out_msg->" << f.name << ".data[idx] = jbin_decode_varint(buffer, &offset);\n";
            } else if (f.type.subTypes[0].name == "string") {
                out << "                    out_msg->" << f.name << ".data[idx] = jbin_decode_string(buffer, &offset);\n";
            } else {
                out << "                    size_t len = jbin_decode_varint(buffer, &offset);\n";
                out << "                    " << f.type.subTypes[0].name << "_unpack(buffer + offset, len, &out_msg->" << f.name << ".data[idx]);\n";
                out << "                    offset += len;\n";
            }
            out << "                }\n";
        } else if (f.type.name == "map") {
            std::string kC = getCType(f.type.subTypes[0], currentSchema);
            std::string vC = getCType(f.type.subTypes[1], currentSchema);
            out << "                {\n";
            out << "                    size_t entry_len = jbin_decode_varint(buffer, &offset);\n";
            out << "                    size_t entry_end = offset + entry_len;\n";
            out << "                    out_msg->" << f.name << ".length++;\n";
            out << "                    out_msg->" << f.name << ".keys = (" << kC << "*)realloc(out_msg->" << f.name << ".keys, out_msg->" << f.name << ".length * sizeof(" << kC << "));\n";
            out << "                    out_msg->" << f.name << ".values = (" << vC << "*)realloc(out_msg->" << f.name << ".values, out_msg->" << f.name << ".length * sizeof(" << vC << "));\n";
            out << "                    size_t idx = out_msg->" << f.name << ".length - 1;\n";
            out << "                    while (offset < entry_end) {\n";
            out << "                        uint32_t etag = jbin_decode_varint(buffer, &offset);\n";
            out << "                        if ((etag >> 3) == 1) {\n";
            if (f.type.subTypes[0].name == "i32" || f.type.subTypes[0].name == "i64") {
                out << "                            out_msg->" << f.name << ".keys[idx] = jbin_decode_varint(buffer, &offset);\n";
            } else {
                out << "                            out_msg->" << f.name << ".keys[idx] = jbin_decode_string(buffer, &offset);\n";
            }
            out << "                        } else if ((etag >> 3) == 2) {\n";
            bool vVarint = (f.type.subTypes[1].name == "i32" || f.type.subTypes[1].name == "i64" || f.type.subTypes[1].name == "bool" || isEnum(f.type.subTypes[1].name, currentSchema));
            if (vVarint) {
                out << "                            out_msg->" << f.name << ".values[idx] = jbin_decode_varint(buffer, &offset);\n";
            } else if (f.type.subTypes[1].name == "string") {
                out << "                            out_msg->" << f.name << ".values[idx] = jbin_decode_string(buffer, &offset);\n";
            } else {
                out << "                            size_t vlen = jbin_decode_varint(buffer, &offset);\n";
                out << "                            " << f.type.subTypes[1].name << "_unpack(buffer + offset, vlen, &out_msg->" << f.name << ".values[idx]);\n";
                out << "                            offset += vlen;\n";
            }
            out << "                        } else {\n";
            out << "                            return false;\n";
            out << "                        }\n";
            out << "                    }\n";
            out << "                }\n";
        } else {
            // Nested message unpack
            out << "                {\n";
            out << "                    size_t len = jbin_decode_varint(buffer, &offset);\n";
            out << "                    " << f.type.name << "_unpack(buffer + offset, len, &out_msg->" << f.name << ");\n";
            out << "                    offset += len;\n";
            out << "                }\n";
        }
        out << "                break;\n";
    }
    out << "            default:\n";
    out << "                return false;\n";
    out << "        }\n";
    out << "    }\n";
    out << "    return true;\n";
    out << "}\n\n";
}

void CGenerator::visit(const Field &field) {
    out << "    " << getCType(field.type, currentSchema) << " " << field.name << ";\n";
}

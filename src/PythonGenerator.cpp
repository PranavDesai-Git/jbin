#include "PythonGenerator.hpp"
#include "Schema.hpp"
#include <iostream>
#include <string>

// Helper to determine if type is an Enum
static bool isEnumPy(const std::string& name, const Schema* schema) {
    if (!schema) return false;
    for (const auto& e : schema->enums) {
        if (e.name == name) return true;
    }
    return false;
}

// Convert schema types to Python type hints
static std::string getPyType(const DataType& type, const Schema* schema) {
    if (type.name == "i32" || type.name == "i64") return "int";
    if (type.name == "string") return "str";
    if (type.name == "bool") return "bool";
    if (type.name == "list") {
        return "List[" + getPyType(type.subTypes[0], schema) + "]";
    }
    if (type.name == "map") {
        return "Dict[" + getPyType(type.subTypes[0], schema) + ", " + getPyType(type.subTypes[1], schema) + "]";
    }
    // Custom messages or enums
    return "'" + type.name + "'";
}

// Get default initialization value for Python dataclasses
static std::string getPyDefault(const DataType& type, const Schema* schema) {
    if (type.name == "i32" || type.name == "i64") return "0";
    if (type.name == "string") return "\"\"";
    if (type.name == "bool") return "False";
    if (type.name == "list") return "field(default_factory=list)";
    if (type.name == "map") return "field(default_factory=dict)";
    if (isEnumPy(type.name, schema)) {
        for (const auto& e : schema->enums) {
            if (e.name == type.name) {
                return type.name + "." + e.entries[0].name; // Default to first enum value
            }
        }
    }
    return "field(default_factory=" + type.name + ")";
}


void PythonGenerator::visit(const Schema &schema) {
    currentSchema = &schema;

    out << "from dataclasses import dataclass, field\n";
    out << "from typing import List, Dict, Optional, Tuple, Any\n";
    out << "from enum import Enum\n";
    out << "import struct\n\n";

    // --- Runtime Helpers ---
    out << "def jbin_encode_varint(value: int) -> bytes:\n"
        << "    res = bytearray()\n"
        << "    while True:\n"
        << "        byte = value & 0x7F\n"
        << "        value >>= 7\n"
        << "        if value:\n"
        << "            res.append(byte | 0x80)\n"
        << "        else:\n"
        << "            res.append(byte)\n"
        << "            break\n"
        << "    return bytes(res)\n\n";

    out << "def jbin_decode_varint(buffer: bytes, offset: int) -> Tuple[int, int]:\n"
        << "    result = 0\n"
        << "    shift = 0\n"
        << "    while True:\n"
        << "        byte = buffer[offset]\n"
        << "        offset += 1\n"
        << "        result |= (byte & 0x7F) << shift\n"
        << "        if not (byte & 0x80):\n"
        << "            break\n"
        << "        shift += 7\n"
        << "    return result, offset\n\n";

    out << "def jbin_encode_zigzag(value: int) -> int:\n"
        << "    return (value << 1) ^ (value >> 63)\n\n";

    out << "def jbin_decode_zigzag(value: int) -> int:\n"
        << "    return (value >> 1) ^ -(value & 1)\n\n";

    out << "def jbin_encode_string(value: str) -> bytes:\n"
        << "    encoded = value.encode('utf-8')\n"
        << "    return jbin_encode_varint(len(encoded)) + encoded\n\n";

    out << "def jbin_decode_string(buffer: bytes, offset: int) -> Tuple[str, int]:\n"
        << "    length, offset = jbin_decode_varint(buffer, offset)\n"
        << "    string_val = buffer[offset:offset+length].decode('utf-8')\n"
        << "    return string_val, offset + length\n\n";

    for (const auto &e : schema.enums) {
        e.accept(*this);
    }
    for (const auto &m : schema.messages) {
        m.accept(*this);
    }
}

void PythonGenerator::visit(const EnumDef &enumDef) {
    currentEnumName = enumDef.name;
    out << "class " << enumDef.name << "(Enum):\n";
    for (const auto &ee : enumDef.entries) {
        ee.accept(*this);
    }
    out << "\n";
}

void PythonGenerator::visit(const EnumEntry &ee) {
    out << "    " << ee.name << " = " << ee.number << "\n";
}

void PythonGenerator::visit(const MessageDef &message) {
    out << "@dataclass\n";
    out << "class " << message.name << ":\n";
    if (message.fields.empty()) {
        out << "    pass\n";
    }
    for (const auto &field : message.fields) {
        field.accept(*this);
    }
    out << "\n";

    // GENERATE PACK
    out << "    def pack(self) -> bytes:\n";
    out << "        buffer = bytearray()\n";
    for (const auto &f : message.fields) {
        bool isE = isEnumPy(f.type.name, currentSchema);
        bool varint = (f.type.name == "i32" || f.type.name == "i64" || f.type.name == "bool" || isE);
        
        if (varint) {
            out << "        buffer.extend(jbin_encode_varint((" << f.number << " << 2) | 0))\n";
            if (f.type.name == "i32" || f.type.name == "i64") {
                out << "        buffer.extend(jbin_encode_varint(jbin_encode_zigzag(self." << f.name << ")))\n";
            } else if (f.type.name == "bool") {
                out << "        buffer.extend(jbin_encode_varint(1 if self." << f.name << " else 0))\n";
            } else { // Enum
                out << "        buffer.extend(jbin_encode_varint(self." << f.name << ".value))\n";
            }
        } else if (f.type.name == "string") {
            out << "        if self." << f.name << ":\n";
            out << "            buffer.extend(jbin_encode_varint((" << f.number << " << 2) | 3))\n";
            out << "            buffer.extend(jbin_encode_string(self." << f.name << "))\n";
        } else if (f.type.name == "list") {
            bool subVarint = (f.type.subTypes[0].name == "i32" || f.type.subTypes[0].name == "i64" || f.type.subTypes[0].name == "bool" || isEnumPy(f.type.subTypes[0].name, currentSchema));
            out << "        for item in self." << f.name << ":\n";
            if (subVarint) {
                out << "            buffer.extend(jbin_encode_varint((" << f.number << " << 2) | 0))\n";
                if (f.type.subTypes[0].name == "i32" || f.type.subTypes[0].name == "i64") {
                    out << "            buffer.extend(jbin_encode_varint(jbin_encode_zigzag(item)))\n";
                } else if (f.type.subTypes[0].name == "bool") {
                    out << "            buffer.extend(jbin_encode_varint(1 if item else 0))\n";
                } else {
                    out << "            buffer.extend(jbin_encode_varint(item.value))\n";
                }
            } else if (f.type.subTypes[0].name == "string") {
                out << "            buffer.extend(jbin_encode_varint((" << f.number << " << 2) | 3))\n";
                out << "            buffer.extend(jbin_encode_string(item))\n";
            } else { // Nested message list
                out << "            temp = item.pack()\n";
                out << "            buffer.extend(jbin_encode_varint((" << f.number << " << 2) | 3))\n";
                out << "            buffer.extend(jbin_encode_varint(len(temp)))\n";
                out << "            buffer.extend(temp)\n";
            }
        } else if (f.type.name == "map") {
            out << "        for k, v in self." << f.name << ".items():\n";
            out << "            temp = bytearray()\n";
            // Map Key
            if (f.type.subTypes[0].name == "i32" || f.type.subTypes[0].name == "i64") {
                out << "            temp.extend(jbin_encode_varint((1 << 2) | 0))\n";
                out << "            temp.extend(jbin_encode_varint(jbin_encode_zigzag(k)))\n";
            } else { // String
                out << "            temp.extend(jbin_encode_varint((1 << 2) | 3))\n";
                out << "            temp.extend(jbin_encode_string(k))\n";
            }
            // Map Value
            bool vVarint = (f.type.subTypes[1].name == "i32" || f.type.subTypes[1].name == "i64" || f.type.subTypes[1].name == "bool" || isEnumPy(f.type.subTypes[1].name, currentSchema));
            if (vVarint) {
                out << "            temp.extend(jbin_encode_varint((2 << 2) | 0))\n";
                if (f.type.subTypes[1].name == "i32" || f.type.subTypes[1].name == "i64") {
                    out << "            temp.extend(jbin_encode_varint(jbin_encode_zigzag(v)))\n";
                } else if (f.type.subTypes[1].name == "bool") {
                    out << "            temp.extend(jbin_encode_varint(1 if v else 0))\n";
                } else { // Enum
                    out << "            temp.extend(jbin_encode_varint(v.value))\n";
                }
            } else if (f.type.subTypes[1].name == "string") {
                out << "            temp.extend(jbin_encode_varint((2 << 2) | 3))\n";
                out << "            temp.extend(jbin_encode_string(v))\n";
            } else { // Nested map value
                out << "            v_temp = v.pack()\n";
                out << "            temp.extend(jbin_encode_varint((2 << 2) | 3))\n";
                out << "            temp.extend(jbin_encode_varint(len(v_temp)))\n";
                out << "            temp.extend(v_temp)\n";
            }
            out << "            buffer.extend(jbin_encode_varint((" << f.number << " << 2) | 3))\n";
            out << "            buffer.extend(jbin_encode_varint(len(temp)))\n";
            out << "            buffer.extend(temp)\n";
        } else {
            // Nested Message
            out << "        temp = self." << f.name << ".pack()\n";
            out << "        buffer.extend(jbin_encode_varint((" << f.number << " << 2) | 3))\n";
            out << "        buffer.extend(jbin_encode_varint(len(temp)))\n";
            out << "        buffer.extend(temp)\n";
        }
    }
    out << "        return bytes(buffer)\n\n";

    // GENERATE UNPACK
    out << "    @classmethod\n";
    out << "    def unpack(cls, buffer: bytes, offset: int = 0) -> Tuple['" << message.name << "', int]:\n";
    out << "        msg = cls()\n";
    out << "        limit = len(buffer)\n";
    out << "        while offset < limit:\n";
    out << "            tag, offset = jbin_decode_varint(buffer, offset)\n";
    out << "            field_num = tag >> 2\n";
    out << "            wire_type = tag & 0x3\n";
    
    // Switch on fields
    bool first = true;
    for (const auto& f : message.fields) {
        if (first) { out << "            if field_num == " << f.number << ":\n"; first = false; }
        else { out << "            elif field_num == " << f.number << ":\n"; }
        
        bool isE = isEnumPy(f.type.name, currentSchema);
        bool varint = (f.type.name == "i32" || f.type.name == "i64" || f.type.name == "bool" || isE);
        
        if (varint) {
            if (f.type.name == "i32" || f.type.name == "i64") {
                out << "                val, offset = jbin_decode_varint(buffer, offset)\n";
                out << "                msg." << f.name << " = jbin_decode_zigzag(val)\n";
            } else if (f.type.name == "bool") {
                out << "                val, offset = jbin_decode_varint(buffer, offset)\n";
                out << "                msg." << f.name << " = bool(val)\n";
            } else { // Enum
                out << "                val, offset = jbin_decode_varint(buffer, offset)\n";
                out << "                msg." << f.name << " = " << f.type.name << "(val)\n";
            }
        } else if (f.type.name == "string") {
            out << "                msg." << f.name << ", offset = jbin_decode_string(buffer, offset)\n";
        } else if (f.type.name == "list") {
            bool subVarint = (f.type.subTypes[0].name == "i32" || f.type.subTypes[0].name == "i64" || f.type.subTypes[0].name == "bool" || isEnumPy(f.type.subTypes[0].name, currentSchema));
            if (subVarint) {
                if (f.type.subTypes[0].name == "i32" || f.type.subTypes[0].name == "i64") {
                    out << "                val, offset = jbin_decode_varint(buffer, offset)\n";
                    out << "                msg." << f.name << ".append(jbin_decode_zigzag(val))\n";
                } else if (f.type.subTypes[0].name == "bool") {
                    out << "                val, offset = jbin_decode_varint(buffer, offset)\n";
                    out << "                msg." << f.name << ".append(bool(val))\n";
                } else {
                    out << "                val, offset = jbin_decode_varint(buffer, offset)\n";
                    out << "                msg." << f.name << ".append(" << f.type.subTypes[0].name << "(val))\n";
                }
            } else if (f.type.subTypes[0].name == "string") {
                out << "                val, offset = jbin_decode_string(buffer, offset)\n";
                out << "                msg." << f.name << ".append(val)\n";
            } else {
                out << "                length, offset = jbin_decode_varint(buffer, offset)\n";
                out << "                val, _ = " << f.type.subTypes[0].name << ".unpack(buffer[offset:offset+length])\n";
                out << "                msg." << f.name << ".append(val)\n";
                out << "                offset += length\n";
            }
        } else if (f.type.name == "map") {
            out << "                length, offset = jbin_decode_varint(buffer, offset)\n";
            out << "                entry_limit = offset + length\n";
            out << "                k, v = None, None\n";
            out << "                while offset < entry_limit:\n";
            out << "                    etag, offset = jbin_decode_varint(buffer, offset)\n";
            out << "                    if (etag >> 2) == 1:\n";
            if (f.type.subTypes[0].name == "i32" || f.type.subTypes[0].name == "i64") {
                out << "                        val, offset = jbin_decode_varint(buffer, offset)\n";
                out << "                        k = jbin_decode_zigzag(val)\n";
            } else {
                out << "                        k, offset = jbin_decode_string(buffer, offset)\n";
            }
            out << "                    elif (etag >> 2) == 2:\n";
            bool vVarint = (f.type.subTypes[1].name == "i32" || f.type.subTypes[1].name == "i64" || f.type.subTypes[1].name == "bool" || isEnumPy(f.type.subTypes[1].name, currentSchema));
            if (vVarint) {
                if (f.type.subTypes[1].name == "i32" || f.type.subTypes[1].name == "i64") {
                    out << "                        val, offset = jbin_decode_varint(buffer, offset)\n";
                    out << "                        v = jbin_decode_zigzag(val)\n";
                } else if (f.type.subTypes[1].name == "bool") {
                    out << "                        val, offset = jbin_decode_varint(buffer, offset)\n";
                    out << "                        v = bool(val)\n";
                } else {
                    out << "                        val, offset = jbin_decode_varint(buffer, offset)\n";
                    out << "                        v = " << f.type.subTypes[1].name << "(val)\n";
                }
            } else if (f.type.subTypes[1].name == "string") {
                out << "                        v, offset = jbin_decode_string(buffer, offset)\n";
            } else {
                out << "                        vlen, offset = jbin_decode_varint(buffer, offset)\n";
                out << "                        v, _ = " << f.type.subTypes[1].name << ".unpack(buffer[offset:offset+vlen])\n";
                out << "                        offset += vlen\n";
            }
            out << "                msg." << f.name << "[k] = v\n";
        } else {
            // Nested Message
            out << "                length, offset = jbin_decode_varint(buffer, offset)\n";
            out << "                msg." << f.name << ", _ = " << f.type.name << ".unpack(buffer[offset:offset+length])\n";
            out << "                offset += length\n";
        }
    }
    if (!message.fields.empty()) {
        out << "            else:\n";
        out << "                if wire_type == 0:\n";
        out << "                    _, offset = jbin_decode_varint(buffer, offset)\n";
        out << "                elif wire_type == 3:\n";
        out << "                    length, offset = jbin_decode_varint(buffer, offset)\n";
        out << "                    offset += length\n";
    } else {
        out << "            if wire_type == 0:\n";
        out << "                _, offset = jbin_decode_varint(buffer, offset)\n";
        out << "            elif wire_type == 3:\n";
        out << "                length, offset = jbin_decode_varint(buffer, offset)\n";
        out << "                offset += length\n";
    }
    
    out << "        return msg, offset\n\n";
}

void PythonGenerator::visit(const Field &field) {
    out << "    " << field.name << ": " << getPyType(field.type, currentSchema) << " = " << getPyDefault(field.type, currentSchema) << "\n";
}

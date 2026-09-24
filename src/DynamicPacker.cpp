#include "DynamicPacker.hpp"
#include "Schema.hpp"
#include "Tag.hpp"
#include "Varint.hpp"
#include <stdexcept>
#include <string>
#include <cstring>

DynamicPacker::DynamicPacker(const Schema &schema) : schema(schema) {}
DynamicReader::DynamicReader(const Schema &schema) : schema(schema) {}

const MessageDef *DynamicPacker::findMessage(const std::string &name) const {
    for (const auto &m : schema.messages)
        if (m.name == name)
            return &m;
    return nullptr;
}
const EnumDef *DynamicPacker::findEnum(const std::string &name) const {
    for (const auto &e : schema.enums)
        if (e.name == name)
            return &e;
    return nullptr;
}

const MessageDef *DynamicReader::findMessage(const std::string &name) const {
    for (const auto &m : schema.messages)
        if (m.name == name)
            return &m;
    return nullptr;
}
const EnumDef *DynamicReader::findEnum(const std::string &name) const {
    for (const auto &e : schema.enums)
        if (e.name == name)
            return &e;
    return nullptr;
}

std::vector<uint8_t>
DynamicPacker::pack(const std::string &messageName,
                    const nlohmann::json &jsonValue) const {
    const MessageDef *msgDef = findMessage(messageName);
    if (!msgDef)
        throw std::runtime_error("Message not found: " + messageName);
    std::vector<uint8_t> buffer;
    packMessage(buffer, *msgDef, jsonValue);
    return buffer;
}

void DynamicPacker::packMessage(std::vector<uint8_t> &buffer,
                                const MessageDef &messageDef,
                                const nlohmann::json &jsonValue) const {
    if (!jsonValue.is_object())
        return;
    for (const Field &field : messageDef.fields) {
        if (jsonValue.contains(field.name)) {
            packValue(buffer, field.number, field.type, jsonValue[field.name]);
        }
    }
}

void DynamicPacker::packValue(std::vector<uint8_t> &buffer,
                              uint32_t fieldNumber, const DataType &type,
                              const nlohmann::json &value) const {
    if (type.name == "i32" || type.name == "i64") {
        encodeTag(buffer, fieldNumber, wiretype::Varint);
        encodeVariant(buffer, encodeZigZag(value.get<int64_t>()));
    } else if (type.name == "f32") {
        encodeTag(buffer, fieldNumber, wiretype::Fixed32);
        float f = value.get<float>();
        uint8_t bytes[4];
        std::memcpy(bytes, &f, 4);
        buffer.insert(buffer.end(), bytes, bytes + 4);
    } else if (type.name == "f64") {
        encodeTag(buffer, fieldNumber, wiretype::Fixed64);
        double d = value.get<double>();
        uint8_t bytes[8];
        std::memcpy(bytes, &d, 8);
        buffer.insert(buffer.end(), bytes, bytes + 8);
    } else if (type.name == "bytes") {
        if (value.is_array()) {
            std::vector<uint8_t> temp;
            for (const auto& b : value) temp.push_back(b.get<uint8_t>());
            encodeTag(buffer, fieldNumber, wiretype::Delimited);
            encodeVariant(buffer, temp.size());
            buffer.insert(buffer.end(), temp.begin(), temp.end());
        } else if (value.is_string()) {
            encodeString(buffer, fieldNumber, value.get<std::string>());
        }
    } else if (type.name == "union") {
        int matchedIndex = -1;
        for (size_t i = 0; i < type.subTypes.size(); i++) {
            const auto& st = type.subTypes[i];
            if ((st.name == "i32" || st.name == "i64" || st.name == "f32" || st.name == "f64") && value.is_number()) {
                matchedIndex = i; break;
            } else if ((st.name == "string" || st.name == "bytes") && value.is_string()) {
                matchedIndex = i; break;
            } else if (st.name == "bool" && value.is_boolean()) {
                matchedIndex = i; break;
            } else if (st.name == "list" && value.is_array()) {
                matchedIndex = i; break;
            } else if (st.name == "map" && value.is_object()) {
                matchedIndex = i; break;
            } else if (findMessage(st.name) && value.is_object()) {
                matchedIndex = i; break;
            } else if (findEnum(st.name) && (value.is_string() || value.is_number_integer())) {
                matchedIndex = i; break;
            }
        }
        if (matchedIndex != -1) {
            std::vector<uint8_t> temp;
            encodeTag(temp, 1, wiretype::Varint);
            encodeVariant(temp, matchedIndex);
            packValue(temp, 2, type.subTypes[matchedIndex], value);
            encodeTag(buffer, fieldNumber, wiretype::Delimited);
            encodeVariant(buffer, temp.size());
            buffer.insert(buffer.end(), temp.begin(), temp.end());
        } else {
            throw std::runtime_error("No matching type in union for value");
        }
    } else if (type.name == "string") {
        encodeString(buffer, fieldNumber, value.get<std::string>());
    } else if (type.name == "bool") {
        encodeTag(buffer, fieldNumber, wiretype::Varint);
        encodeVariant(buffer, value.get<bool>() ? 1 : 0);
    } else if (type.name == "list") {
        if (!value.is_array())
            throw std::runtime_error("Expected JSON array for list");
        for (const auto &item : value) {
            packValue(buffer, fieldNumber, type.subTypes[0], item);
        }
    } else if (type.name == "map") {
        if (!value.is_object())
            throw std::runtime_error("Expected JSON object for map");
        for (auto it = value.begin(); it != value.end(); ++it) {
            std::vector<uint8_t> entryBuffer;

            // Map keys in JSON are always strings. If schema key is int, parse
            // it.
            nlohmann::json keyJson = it.key();
            if (type.subTypes[0].name == "i32" ||
                type.subTypes[0].name == "i64") {
                keyJson = std::stoll(it.key());
            }

            packValue(entryBuffer, 1, type.subTypes[0], keyJson);
            packValue(entryBuffer, 2, type.subTypes[1], it.value());

            encodeTag(buffer, fieldNumber, wiretype::Delimited);
            encodeVariant(buffer, entryBuffer.size());
            buffer.insert(buffer.end(), entryBuffer.begin(), entryBuffer.end());
        }
    } else {
        const MessageDef *nestedMsgDef = findMessage(type.name);
        const EnumDef *enumDef = findEnum(type.name);

        if (nestedMsgDef) {
            std::vector<uint8_t> tempBuffer;
            packMessage(tempBuffer, *nestedMsgDef, value);
            encodeTag(buffer, fieldNumber, wiretype::Delimited);
            encodeVariant(buffer, tempBuffer.size());
            buffer.insert(buffer.end(), tempBuffer.begin(), tempBuffer.end());
        } else if (enumDef) {
            uint32_t enumVal = 0;
            if (value.is_string()) {
                std::string s = value.get<std::string>();
                for (const auto &e : enumDef->entries) {
                    if (e.name == s)
                        enumVal = e.number;
                }
            } else if (value.is_number_integer()) {
                enumVal = value.get<uint32_t>();
            }
            encodeTag(buffer, fieldNumber, wiretype::Varint);
            encodeVariant(buffer, enumVal);
        } else {
            throw std::runtime_error("Packing for type " + type.name +
                                     " not implemented");
        }
    }
}

nlohmann::json DynamicReader::unpack(const std::string &messageName,
                                     const std::vector<uint8_t> &buffer) const {
    const MessageDef *msgDef = findMessage(messageName);
    if (!msgDef)
        throw std::runtime_error("Message not found: " + messageName);
    size_t offset = 0;
    return unpackMessage(buffer, offset, *msgDef, buffer.size());
}

nlohmann::json DynamicReader::unpackMessage(const std::vector<uint8_t> &buffer,
                                            size_t &offset,
                                            const MessageDef &messageDef,
                                            size_t limit) const {
    nlohmann::json result = nlohmann::json::object();
    while (offset < limit) {
        uint32_t fieldNumber;
        wiretype wt;
        decodeTag(buffer, offset, fieldNumber, wt);

        const Field *foundField = nullptr;
        for (const auto &f : messageDef.fields) {
            if (f.number == fieldNumber) {
                foundField = &f;
                break;
            }
        }

        if (foundField) {
            if (foundField->type.name == "list") {
                if (!result.contains(foundField->name)) {
                    result[foundField->name] = nlohmann::json::array();
                }
                result[foundField->name].push_back(unpackValue(
                    buffer, offset, foundField->type.subTypes[0], wt));
            } else if (foundField->type.name == "map") {
                if (!result.contains(foundField->name)) {
                    result[foundField->name] = nlohmann::json::object();
                }
                uint64_t length = decodeVariant(buffer, offset);
                size_t entryLimit = offset + length;
                nlohmann::json keyJson, valJson;
                while (offset < entryLimit) {
                    uint32_t fNum;
                    wiretype fWt;
                    decodeTag(buffer, offset, fNum, fWt);
                    if (fNum == 1)
                        keyJson = unpackValue(
                            buffer, offset, foundField->type.subTypes[0], fWt);
                    else if (fNum == 2)
                        valJson = unpackValue(
                            buffer, offset, foundField->type.subTypes[1], fWt);
                    else {
                        if (fWt == wiretype::Varint)
                            decodeVariant(buffer, offset);
                        else if (fWt == wiretype::Delimited) {
                            uint64_t l = decodeVariant(buffer, offset);
                            offset += l;
                        } else if (fWt == wiretype::Fixed32) {
                            offset += 4;
                        } else if (fWt == wiretype::Fixed64) {
                            offset += 8;
                        }
                    }
                }
                std::string keyStr = keyJson.is_string()
                                         ? keyJson.get<std::string>()
                                         : keyJson.dump();
                result[foundField->name][keyStr] = valJson;
            } else {
                result[foundField->name] =
                    unpackValue(buffer, offset, foundField->type, wt);
            }
        } else {
            if (wt == wiretype::Varint)
                decodeVariant(buffer, offset);
            else if (wt == wiretype::Delimited) {
                uint64_t l = decodeVariant(buffer, offset);
                offset += l;
            } else if (wt == wiretype::Fixed32) {
                offset += 4;
            } else if (wt == wiretype::Fixed64) {
                offset += 8;
            }
        }
    }
    return result;
}

nlohmann::json DynamicReader::unpackValue(const std::vector<uint8_t> &buffer,
                                          size_t &offset, const DataType &type,
                                          wiretype wt) const {
    if (type.name == "i32" || type.name == "i64") {
        return decodeZigZag(decodeVariant(buffer, offset));
    } else if (type.name == "f32") {
        float f;
        std::memcpy(&f, buffer.data() + offset, 4);
        offset += 4;
        return f;
    } else if (type.name == "f64") {
        double d;
        std::memcpy(&d, buffer.data() + offset, 8);
        offset += 8;
        return d;
    } else if (type.name == "bytes") {
        uint64_t length = decodeVariant(buffer, offset);
        std::vector<uint8_t> bytes(buffer.begin() + offset, buffer.begin() + offset + length);
        offset += length;
        return bytes;
    } else if (type.name == "string") {
        return decodeString(buffer, offset);
    } else if (type.name == "bool") {
        return decodeVariant(buffer, offset) != 0;
    } else if (type.name == "union") {
        uint64_t length = decodeVariant(buffer, offset);
        size_t limit = offset + length;
        uint32_t typeIndex = 0;
        nlohmann::json val;
        while (offset < limit) {
            uint32_t fNum;
            wiretype fWt;
            decodeTag(buffer, offset, fNum, fWt);
            if (fNum == 1) {
                typeIndex = decodeVariant(buffer, offset);
            } else if (fNum == 2) {
                if (typeIndex < type.subTypes.size()) {
                    val = unpackValue(buffer, offset, type.subTypes[typeIndex], fWt);
                } else {
                    throw std::runtime_error("Invalid type index in union");
                }
            } else {
                if (fWt == wiretype::Varint) decodeVariant(buffer, offset);
                else if (fWt == wiretype::Delimited) offset += decodeVariant(buffer, offset);
                else if (fWt == wiretype::Fixed32) offset += 4;
                else if (fWt == wiretype::Fixed64) offset += 8;
            }
        }
        return val;
    } else {
        const MessageDef *nestedMsgDef = findMessage(type.name);
        const EnumDef *enumDef = findEnum(type.name);

        if (nestedMsgDef) {
            uint64_t length = decodeVariant(buffer, offset);
            size_t childLimit = offset + length;
            return unpackMessage(buffer, offset, *nestedMsgDef, childLimit);
        } else if (enumDef) {
            uint64_t val = decodeVariant(buffer, offset);
            for (const auto &e : enumDef->entries) {
                if (e.number == val)
                    return e.name;
            }
            return val;
        } else {
            throw std::runtime_error("Unpacking for type " + type.name +
                                     " not implemented");
        }
    }
}

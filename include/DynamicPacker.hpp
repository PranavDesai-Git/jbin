#pragma once

#include "Schema.hpp"
#include "Tag.hpp"
#include <cstdint>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

class DynamicPacker {
  public:
    DynamicPacker(const Schema &schema);

    std::vector<uint8_t> pack(const std::string &messageName,
                              const nlohmann::json &jsonValue) const;

  private:
    const Schema &schema;

    const MessageDef *findMessage(const std::string &name) const;
    const EnumDef *findEnum(const std::string &name) const;

    void packMessage(std::vector<uint8_t> &buffer, const MessageDef &messageDef,
                     const nlohmann::json &jsonValue) const;

    void packValue(std::vector<uint8_t> &buffer, uint32_t fieldNumber,
                   const DataType &type, const nlohmann::json &value) const;
};

class DynamicReader {
  public:
    DynamicReader(const Schema &schema);

    nlohmann::json unpack(const std::string &messageName,
                          const std::vector<uint8_t> &buffer) const;

  private:
    const Schema &schema;

    const MessageDef *findMessage(const std::string &name) const;
    const EnumDef *findEnum(const std::string &name) const;
    
    nlohmann::json unpackMessage(const std::vector<uint8_t> &buffer,
                                 size_t &offset, const MessageDef &messageDef,
                                 size_t limit) const;

    nlohmann::json unpackValue(const std::vector<uint8_t> &buffer,
                               size_t &offset, const DataType &type,
                               wiretype wt) const;
};

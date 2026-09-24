#pragma once
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

static inline void jbin_encode_varint(uint8_t* buffer, size_t* offset, uint64_t value) {
    while (value >= 0x80) {
        buffer[(*offset)++] = (value & 0x7F) | 0x80;
        value >>= 7;
    }
    buffer[(*offset)++] = (uint8_t)(value & 0x7F);
}

static inline uint64_t jbin_decode_varint(const uint8_t* buffer, size_t* offset) {
    uint64_t result = 0;
    int shift = 0;
    while (1) {
        uint8_t byte = buffer[(*offset)++];
        result |= (uint64_t)(byte & 0x7F) << shift;
        if (!(byte & 0x80)) break;
        shift += 7;
    }
    return result;
}

static inline void jbin_encode_string(uint8_t* buffer, size_t* offset, const char* str) {
    size_t len = strlen(str);
    jbin_encode_varint(buffer, offset, len);
    memcpy(buffer + (*offset), str, len);
    *offset += len;
}

static inline char* jbin_decode_string(const uint8_t* buffer, size_t* offset) {
    size_t len = jbin_decode_varint(buffer, offset);
    char* str = (char*)malloc(len + 1);
    memcpy(str, buffer + (*offset), len);
    str[len] = '\0';
    *offset += len;
    return str;
}

typedef struct Vector3 Vector3;
typedef struct InventoryItem InventoryItem;
typedef struct Character Character;

typedef struct {
    InventoryItem* data;
    size_t length;
    size_t capacity;
} jbin_list_InventoryItem;

typedef struct {
    char** keys;
    int32_t* values;
    size_t length;
    size_t capacity;
} jbin_map_char_ptr_int32;

typedef enum {
    Faction_alliance = 1,
    Faction_horde = 2,
    Faction_neutral = 3,
} Faction;

struct Vector3 {
    int32_t x;
    int32_t y;
    int32_t z;
};

int32_t Vector3_get_x(const Vector3* msg) {
    return msg->x;
}

void Vector3_set_x(Vector3* msg, int32_t val) {
    msg->x = val;
}

int32_t Vector3_get_y(const Vector3* msg) {
    return msg->y;
}

void Vector3_set_y(Vector3* msg, int32_t val) {
    msg->y = val;
}

int32_t Vector3_get_z(const Vector3* msg) {
    return msg->z;
}

void Vector3_set_z(Vector3* msg, int32_t val) {
    msg->z = val;
}

size_t Vector3_pack(const Vector3* msg, uint8_t* buffer) {
    size_t offset = 0;
    // Field x
    jbin_encode_varint(buffer, &offset, (1 << 3) | 0);
    jbin_encode_varint(buffer, &offset, (uint64_t)msg->x);
    // Field y
    jbin_encode_varint(buffer, &offset, (2 << 3) | 0);
    jbin_encode_varint(buffer, &offset, (uint64_t)msg->y);
    // Field z
    jbin_encode_varint(buffer, &offset, (3 << 3) | 0);
    jbin_encode_varint(buffer, &offset, (uint64_t)msg->z);
    return offset;
}

bool Vector3_unpack(const uint8_t *buffer, size_t length, Vector3* out_msg) {
    size_t offset = 0;
    while(offset < length) {
        uint32_t tag = jbin_decode_varint(buffer, &offset);
        uint32_t field_num = tag >> 3;
        switch(field_num) {
            case 1:
                out_msg->x = jbin_decode_varint(buffer, &offset);
                break;
            case 2:
                out_msg->y = jbin_decode_varint(buffer, &offset);
                break;
            case 3:
                out_msg->z = jbin_decode_varint(buffer, &offset);
                break;
            default:
                return false;
        }
    }
    return true;
}

struct InventoryItem {
    int32_t itemId;
    int32_t quantity;
    bool isSoulbound;
};

int32_t InventoryItem_get_itemId(const InventoryItem* msg) {
    return msg->itemId;
}

void InventoryItem_set_itemId(InventoryItem* msg, int32_t val) {
    msg->itemId = val;
}

int32_t InventoryItem_get_quantity(const InventoryItem* msg) {
    return msg->quantity;
}

void InventoryItem_set_quantity(InventoryItem* msg, int32_t val) {
    msg->quantity = val;
}

bool InventoryItem_get_isSoulbound(const InventoryItem* msg) {
    return msg->isSoulbound;
}

void InventoryItem_set_isSoulbound(InventoryItem* msg, bool val) {
    msg->isSoulbound = val;
}

size_t InventoryItem_pack(const InventoryItem* msg, uint8_t* buffer) {
    size_t offset = 0;
    // Field itemId
    jbin_encode_varint(buffer, &offset, (1 << 3) | 0);
    jbin_encode_varint(buffer, &offset, (uint64_t)msg->itemId);
    // Field quantity
    jbin_encode_varint(buffer, &offset, (2 << 3) | 0);
    jbin_encode_varint(buffer, &offset, (uint64_t)msg->quantity);
    // Field isSoulbound
    jbin_encode_varint(buffer, &offset, (3 << 3) | 0);
    jbin_encode_varint(buffer, &offset, (uint64_t)msg->isSoulbound);
    return offset;
}

bool InventoryItem_unpack(const uint8_t *buffer, size_t length, InventoryItem* out_msg) {
    size_t offset = 0;
    while(offset < length) {
        uint32_t tag = jbin_decode_varint(buffer, &offset);
        uint32_t field_num = tag >> 3;
        switch(field_num) {
            case 1:
                out_msg->itemId = jbin_decode_varint(buffer, &offset);
                break;
            case 2:
                out_msg->quantity = jbin_decode_varint(buffer, &offset);
                break;
            case 3:
                out_msg->isSoulbound = jbin_decode_varint(buffer, &offset);
                break;
            default:
                return false;
        }
    }
    return true;
}

struct Character {
    int64_t id;
    char* name;
    int32_t level;
    Faction faction;
    Vector3 position;
    jbin_list_InventoryItem inventory;
    jbin_map_char_ptr_int32 attributes;
};

int64_t Character_get_id(const Character* msg) {
    return msg->id;
}

void Character_set_id(Character* msg, int64_t val) {
    msg->id = val;
}

char* Character_get_name(const Character* msg) {
    return msg->name;
}

void Character_set_name(Character* msg, char* val) {
    if (msg->name) free(msg->name);
    msg->name = strdup(val);
}

int32_t Character_get_level(const Character* msg) {
    return msg->level;
}

void Character_set_level(Character* msg, int32_t val) {
    msg->level = val;
}

Faction Character_get_faction(const Character* msg) {
    return msg->faction;
}

void Character_set_faction(Character* msg, Faction val) {
    msg->faction = val;
}

Vector3 Character_get_position(const Character* msg) {
    return msg->position;
}

void Character_set_position(Character* msg, Vector3 val) {
    msg->position = val;
}

jbin_list_InventoryItem Character_get_inventory(const Character* msg) {
    return msg->inventory;
}

void Character_set_inventory(Character* msg, jbin_list_InventoryItem val) {
    msg->inventory = val;
}

jbin_map_char_ptr_int32 Character_get_attributes(const Character* msg) {
    return msg->attributes;
}

void Character_set_attributes(Character* msg, jbin_map_char_ptr_int32 val) {
    msg->attributes = val;
}

size_t Character_pack(const Character* msg, uint8_t* buffer) {
    size_t offset = 0;
    // Field id
    jbin_encode_varint(buffer, &offset, (1 << 3) | 0);
    jbin_encode_varint(buffer, &offset, (uint64_t)msg->id);
    // Field name
    if (msg->name) {
        jbin_encode_varint(buffer, &offset, (2 << 3) | 3);
        jbin_encode_string(buffer, &offset, msg->name);
    }
    // Field level
    jbin_encode_varint(buffer, &offset, (3 << 3) | 0);
    jbin_encode_varint(buffer, &offset, (uint64_t)msg->level);
    // Field faction
    jbin_encode_varint(buffer, &offset, (4 << 3) | 0);
    jbin_encode_varint(buffer, &offset, (uint64_t)msg->faction);
    // Field position
    {
        uint8_t* temp = (uint8_t*)malloc(4096);
        size_t temp_len = Vector3_pack(&msg->position, temp);
        jbin_encode_varint(buffer, &offset, (5 << 3) | 3);
        jbin_encode_varint(buffer, &offset, temp_len);
        memcpy(buffer + offset, temp, temp_len);
        offset += temp_len;
        free(temp);
    }
    // Field inventory
    for (size_t i = 0; i < msg->inventory.length; i++) {
        uint8_t* temp = (uint8_t*)malloc(4096);
        size_t temp_len = InventoryItem_pack(&msg->inventory.data[i], temp);
        jbin_encode_varint(buffer, &offset, (6 << 3) | 3);
        jbin_encode_varint(buffer, &offset, temp_len);
        memcpy(buffer + offset, temp, temp_len);
        offset += temp_len;
        free(temp);
    }
    // Field attributes
    for (size_t i = 0; i < msg->attributes.length; i++) {
        uint8_t* temp = (uint8_t*)malloc(4096);
        size_t temp_off = 0;
        jbin_encode_varint(temp, &temp_off, (1 << 3) | 3);
        jbin_encode_string(temp, &temp_off, msg->attributes.keys[i]);
        jbin_encode_varint(temp, &temp_off, (2 << 3) | 0);
        jbin_encode_varint(temp, &temp_off, (uint64_t)msg->attributes.values[i]);
        jbin_encode_varint(buffer, &offset, (7 << 3) | 3);
        jbin_encode_varint(buffer, &offset, temp_off);
        memcpy(buffer + offset, temp, temp_off);
        offset += temp_off;
        free(temp);
    }
    return offset;
}

bool Character_unpack(const uint8_t *buffer, size_t length, Character* out_msg) {
    size_t offset = 0;
    while(offset < length) {
        uint32_t tag = jbin_decode_varint(buffer, &offset);
        uint32_t field_num = tag >> 3;
        switch(field_num) {
            case 1:
                out_msg->id = jbin_decode_varint(buffer, &offset);
                break;
            case 2:
                out_msg->name = jbin_decode_string(buffer, &offset);
                break;
            case 3:
                out_msg->level = jbin_decode_varint(buffer, &offset);
                break;
            case 4:
                out_msg->faction = jbin_decode_varint(buffer, &offset);
                break;
            case 5:
                {
                    size_t len = jbin_decode_varint(buffer, &offset);
                    Vector3_unpack(buffer + offset, len, &out_msg->position);
                    offset += len;
                }
                break;
            case 6:
                {
                    out_msg->inventory.length++;
                    out_msg->inventory.data = (InventoryItem*)realloc(out_msg->inventory.data, out_msg->inventory.length * sizeof(InventoryItem));
                    size_t idx = out_msg->inventory.length - 1;
                    size_t len = jbin_decode_varint(buffer, &offset);
                    InventoryItem_unpack(buffer + offset, len, &out_msg->inventory.data[idx]);
                    offset += len;
                }
                break;
            case 7:
                {
                    size_t entry_len = jbin_decode_varint(buffer, &offset);
                    size_t entry_end = offset + entry_len;
                    out_msg->attributes.length++;
                    out_msg->attributes.keys = (char**)realloc(out_msg->attributes.keys, out_msg->attributes.length * sizeof(char*));
                    out_msg->attributes.values = (int32_t*)realloc(out_msg->attributes.values, out_msg->attributes.length * sizeof(int32_t));
                    size_t idx = out_msg->attributes.length - 1;
                    while (offset < entry_end) {
                        uint32_t etag = jbin_decode_varint(buffer, &offset);
                        if ((etag >> 3) == 1) {
                            out_msg->attributes.keys[idx] = jbin_decode_string(buffer, &offset);
                        } else if ((etag >> 3) == 2) {
                            out_msg->attributes.values[idx] = jbin_decode_varint(buffer, &offset);
                        } else {
                            return false;
                        }
                    }
                }
                break;
            default:
                return false;
        }
    }
    return true;
}


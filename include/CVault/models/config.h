#ifndef META_CONFIG_H
#define META_CONFIG_H

#include <stdint.h>

/**
 * @brief: Represents a key-value configuration pair.
 * This structure is used to store application level settings, Since values
 * can be binary data, it uses a uint8_t pointer and an explicit length field
 */
typedef struct {
    char *config_key;
    uint8_t *config_value;
    uint32_t config_value_len;
} Config;

#endif

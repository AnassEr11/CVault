#ifndef VAULT_ENTRY_H
#define VAULT_ENTRY_H

#include <stdint.h>

/**
 * @brief Internal vault entry representation.
 *
 * this structure stores raw binary buffers, along with their respective
 * lengths. it is meant for the repository layer
 *
 * @note:
 * - uuid is a null-terminated C string.
 * - other text fields are binary blobs and may contain null bytes.
 * - *_len fields indicate the size of each blob.
 */
typedef struct {
    char *uuid;

    uint8_t *service_name;
    uint8_t *username;
    uint8_t *password;
    uint8_t *notes;

    uint32_t service_len;
    uint32_t username_len;
    uint32_t password_len;
    uint32_t notes_len;

    uint64_t created_at;
    uint64_t updated_at;
} IntVaultEntry;

/**
 * @brief External vault entry representation.
 *
 * This structure stores actual text fields as null-terminated C strings and
 * is used at the UI/interface layer.
 *
 */
typedef struct {
    char *uuid;

    char *service_name;
    char *username;
    char *password;
    char *notes;

    uint64_t created_at;
    uint64_t updated_at;
} ExtVaultEntry;
#endif

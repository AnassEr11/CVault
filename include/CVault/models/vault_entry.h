#ifndef VAULT_ENTRY_H
#define VAULT_ENTRY_H

#include <stdint.h>

/**
 * @brief: Internal Repository Model.
 * used for database operations and internal logic, All string fields
 * are standard null-terminated C strings
 *
 * @note: Memory for pointers should typically be managed (allocated/freed)
 * by the service layer
 */
typedef struct {
    char *uuid;

    char *service_name;
    char *username;
    char *password;
    char *notes;

    uint64_t created_at;
    uint64_t updated_at;
} IntVaultEntry;

/**
 * @brief: External Interface Model
 * used for passing data between the UI and the Crypto Core, Fields are
 * stored as uint8_t buffers to store encrypted binary data (blobs) which
 * may contain null bytes.
 *
 * @warning: Always use the associated _len fields when reading data,
 * as these buffers are not guaranteed to be null-terminated.
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
} ExtVaultEntry;

#endif

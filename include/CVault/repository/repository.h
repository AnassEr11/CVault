#ifndef REPOSITORY_H
#define REPOSITORY_H

#include <CVault/models/config.h>
#include <CVault/models/vault_entry.h>
#include <CVault/utils/data_structure_utils.h>
#include <stdbool.h>
#include <vendor/sqlite3/sqlite3.h>

/**
 * this enum represent all states a repo function may crach for
 */
typedef enum {
    OK = 0,
    DATA_STRUCTURE_ERR,
    DATA_BASE_ERR,
    NOT_FOUND_ERR,
    MEMORY_ERR,
    REPO_UNEXPECTED_ERR
} repo_return_code;

/**
 * @brief Initialize the vault database schema
 *
 * @param db Pointer to the SQLite database connection
 *
 * @return repo_return_code OK on success, DATA_BASE_ERR on database erorr
 */
repo_return_code repo_vault_init(sqlite3 *db);

/**
 * @brief Add a new vault entry to the repository
 *
 * @details Inserts a new entry with encrypted fields into the entries table
 *
 * @param entry Pointer to the vault entry to add
 * @param db Pointer to the SQLite database connection
 *
 * @return repo_return_code OK on success, DATA_BASE_ERR on database error
 */
repo_return_code add_entry(IntVaultEntry *entry, sqlite3 *db);

/**
 * @brief Retrieve a vault entry by UUID
 *
 * @details Fetches a single entry from the database and decrypts its fields
 *
 * @param uuid The unique identifier of the entry to retrieve
 * @param out_entry Pointer to store the retrieved entry data (caller allocated)
 * @param db Pointer to the SQLite database connection
 *
 * @return repo_return_code OK on success, NOT_FOUND_ERR if uuid doesn't exist,
 * MEMORY_ERR on allocation failure, DATA_BASE_ERR on database error
 */
repo_return_code read_entry(const char *uuid, IntVaultEntry *out_entry, sqlite3 *db);

/**
 * @brief Retrieve all vault entries from the repository
 *
 * @details Fetches all entries and stores them in a doubly linked list
 *
 * @param out_wrapper Pointer to the linked list head where entries will be appended (caller
 * allocated)
 * @param db Pointer to the SQLite database connection
 *
 * @return repo_return_code OK on success, MEMORY_ERR on allocation failure,
 * DATA_BASE_ERR on database error, DATA_STRUCTURE_ERR on list operation
 * failure
 */
repo_return_code read_all_entries(DLinkedList *out_wrapper, sqlite3 *db);

/**
 * @brief Update an existing vault entry
 *
 * @details Updates specific fields of an entry identified by UUID. NULL values are skipped.
 *
 * @param uuid The unique identifier of the entry to update
 * @param new_entry Pointer to entry data with updated fields (NULL fields are not updated)
 * @param db Pointer to the SQLite database connection
 *
 * @return repo_return_code OK on success, NOT_FOUND_ERR if uuid doesn't exist,
 * DATA_BASE_ERR on database error, REPO_UNEXPECTED_ERR on unexpected state
 */
repo_return_code update_entry(const char *uuid, IntVaultEntry *new_entry, sqlite3 *db);

/**
 * @brief Delete a vault entry by UUID
 * @details Permanently removes an entry from the database
 * @param uuid The unique identifier of the entry to delete
 * @param db Pointer to the SQLite database connection
 * @return repo_return_code OK on success, NOT_FOUND_ERR if uuid doesn't exist,
 * DATA_BASE_ERR on database error, REPO_UNEXPECTED_ERR on unexpected state
 */
repo_return_code delete_entry(const char *uuid, sqlite3 *db);

/**
 * @brief Delete all vault entries from the repository
 *
 * @details Removes all entries from the entries table
 *
 * @param db Pointer to the SQLite database connection
 *
 * @return repo_return_code OK on success, DATA_BASE_ERR on database error
 */
repo_return_code delete_all_entries(sqlite3 *db);

/**
 * @brief Initialize the config schema
 *
 * @param db Pointer to the SQLite database connection
 *
 * @return repo_return_code OK on success, DATA_BASE_ERR on database error
 */
repo_return_code repo_config_init(sqlite3 *db);

/**
 * @brief Add a new configuration to the repository
 *
 * @details Inserts a new configuration key-value pair into the configs table
 *
 * @param config Pointer to the configuration object to add
 * @param db Pointer to the SQLite database connection
 *
 * @return repo_return_code OK on success, DATA_BASE_ERR on database error
 */
repo_return_code add_config(Config *config, sqlite3 *db);

/**
 * @brief Retrieve a configuration by key
 *
 * @details Fetches a single configuration entry from the database
 *
 * @param key The configuration key to retrieve
 * @param out_config Pointer to store the retrieved configuration (caller allocated)
 * @param db Pointer to the SQLite database connection
 *
 * @return repo_return_code OK on success, NOT_FOUND_ERR if key doesn't exist,
 * MEMORY_ERR on allocation failure, DATA_BASE_ERR on database error
 */
repo_return_code read_config(const char *key, Config *out_config, sqlite3 *db);

/**
 * @brief Update an existing configuration
 *
 * @details Updates the configuration entry identified by key
 *
 * @param key The configuration key to update
 * @param new_config Pointer to the updated configuration data
 * @param db Pointer to the SQLite database connection
 *
 * @return repo_return_code OK on success, NOT_FOUND_ERR if key doesn't exist,
 * DATA_BASE_ERR on database error
 */
repo_return_code update_config(const char *key, Config *new_config, sqlite3 *db);

/**
 * @brief Delete a configuration by key
 *
 * @details Permanently removes a configuration entry from the database
 *
 * @param key The configuration key to delete
 * @param db Pointer to the SQLite database connection
 *
 * @return repo_return_code OK on success, NOT_FOUND_ERR if key doesn't exist,
 * DATA_BASE_ERR on database error
 */
repo_return_code delete_configs(const char *key, sqlite3 *db);

/**
 * @brief Delete all configurations from the repository
 *
 * @details Removes all configuration entries from the configs table
 *
 * @param db Pointer to the SQLite database connection
 *
 * @return repo_return_code OK on success, DATA_BASE_ERR on database error
 */
repo_return_code delete_all_configs(sqlite3 *db);
#endif

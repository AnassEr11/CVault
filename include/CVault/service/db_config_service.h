#ifndef DB_CONFIG_SERVICE_H
#define DB_CONFIG_SERVICE_H

#include <CVault/models/config.h>
#include <CVault/repository/repository.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/**
 * @defgroup ConfigService Configuration Service
 * @brief Service layer for managing application configuration settings
 *
 * @details The Configuration Service provides a high-level interface for managing
 * key-value configuration pairs stored in the SQLite database. It wraps the repository
 * layer and provides validation, verification, and error handling for configuration
 * operations.
 *
 * This service handles:
 * - Database initialization and connection management
 * - Configuration CRUD operations (Create, Read, Update, Delete)
 * - Input validation and error checking
 * - Secure update verification using constant-time comparison
 * - Proper resource cleanup and memory management
 *
 * All operations support binary values through the Config structure's uint8_t pointer
 * and explicit length field, allowing storage of non-text configuration data.
 *
 * @{
 */

/**
 * @brief Initialize the configuration service and open the database connection
 *
 * @details Opens the SQLite database connection for configuration operations.
 * Must be called before any other configuration service functions.
 * Initializes necessary file paths through the environment service.
 *
 * @return bool true if the database connection was successfully opened, false otherwise
 *
 * @note The database connection is stored in a static variable. Only one connection
 * should be active at a time.
 *
 * @see close_config_service()
 */
bool open_config_service();

/**
 * @brief Add a new configuration key-value pair to the repository
 *
 * @details Inserts a new configuration entry into the database. The configuration
 * must contain a valid key, value pointer, and non-zero value length.
 *
 * @param[in] config Pointer to the Config structure containing the key-value pair to add.
 *                   Must not be NULL. config->config_key and config->config_value
 *                   must not be NULL, and config->config_value_len must be > 0.
 *
 * @return bool true if the configuration was successfully added, false if:
 *         - config pointer is NULL
 *         - config->config_key is NULL
 *         - config->config_value is NULL
 *         - config->config_value_len is 0
 *         - the repository operation fails
 *
 * @see service_read_config()
 * @see service_update_config()
 */
bool service_add_config(Config *config);

/**
 * @brief Retrieve a configuration value by its key
 *
 * @details Fetches a configuration entry from the database by key. The retrieved
 * configuration data is copied into the output buffer provided by the caller.
 *
 * @param[in] key The configuration key to retrieve. Must not be NULL.
 * @param[out] out_config Pointer to a Config structure where the retrieved configuration
 *                        will be stored. Must be a valid pointer to a Config structure
 *                        allocated by the caller. The service will populate config_key,
 *                        config_value, and config_value_len fields.
 *
 * @return bool true if the configuration was successfully retrieved and validated,
 *         false if:
 *         - key pointer is NULL
 *         - out_config pointer is NULL
 *         - the configuration key does not exist in the database
 *         - the repository operation fails
 *         - retrieved data has invalid or missing fields
 *
 * @note The caller is responsible for freeing the memory allocated by this function
 * in the out_config structure (config_key and config_value pointers).
 *
 * @see service_add_config()
 * @see service_update_config()
 */
bool service_read_config(char *key, Config *out_config);

/**
 * @brief Update an existing configuration's value
 *
 * @details Updates the value for a configuration identified by its key. After the update,
 * the new value is read back and verified using constant-time comparison to ensure
 * the update was successful.
 *
 * @param[in] key The configuration key to update. Must not be NULL.
 * @param[in] new_value Pointer to the new configuration value (binary data).
 *                      Must not be NULL.
 * @param[in] new_size Size of the new configuration value in bytes. Must be > 0.
 *
 * @return bool true if the configuration was successfully updated and verified,
 *         false if:
 *         - key pointer is NULL
 *         - new_value pointer is NULL
 *         - new_size is 0
 *         - the repository update operation fails
 *         - verification of the update fails (size mismatch or data mismatch)
 *         - memory allocation fails
 *
 * @details The verification process uses constant-time comparison to prevent
 * timing-based side-channel attacks. Memory allocated during verification is properly
 * freed before returning.
 *
 * @see service_read_config()
 * @see service_add_config()
 */
bool service_update_config(char *key, uint8_t *new_value, size_t new_size);

/**
 * @brief Delete a specific configuration by key
 *
 * @details Removes a configuration entry from the database. After deletion, the key
 * is verified to be absent to ensure the delete operation was successful.
 *
 * @param[in] key The configuration key to delete. Must not be NULL.
 *
 * @return bool true if the configuration was successfully deleted and verified as absent,
 *         false if:
 *         - key pointer is NULL
 *         - the configuration key does not exist (NOT_FOUND_ERR treated as success)
 *         - the repository delete operation fails
 *         - verification shows the key still exists after deletion
 *         - memory allocation fails
 *
 * @note This operation performs a verification read to ensure the key no longer exists
 * in the database, providing additional certainty that the delete was successful.
 *
 * @see service_delete_all_configs()
 */
bool service_delete_config(char *key);

/**
 * @brief Delete all configurations from the repository
 *
 * @details Removes all configuration entries from the database. This is a bulk
 * operation that clears the entire configurations table.
 *
 * @return bool true if all configurations were successfully deleted, false if:
 *         - the repository delete operation fails
 *         - a database error occurs
 *
 * @warning This operation is irreversible and removes all stored configurations.
 * Use with caution in production environments.
 *
 * @see service_delete_config()
 */
bool service_delete_all_configs();

/**
 * @brief Close the configuration service and database connection
 *
 * @details Closes the SQLite database connection. Should be called when configuration
 * operations are complete to ensure proper resource cleanup.
 *
 * @return bool true if the database was successfully closed, false if a close error occurred
 *
 * @note After calling this function, no configuration service functions should be called
 * without calling open_config_service() first.
 *
 * @see open_config_service()
 */
bool close_config_service();

/** @} */

#endif // !DB_CONFIG_SERVICE_H

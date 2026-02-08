#ifndef ENVIRONMENT_SERVICE_H
#define ENVIRONMENT_SERVICE_H

#include <stdbool.h>
#include <stddef.h>

#define DB_CONFIG_FILE "config.db"
#define CONFIG_HOME_PATH ".config/cvault"
#define DB_VAULT_FILE "vault.db"
#define TITAN_KEY_FILE "titan.key"

#define ENV_CONFIG_PATH "CVAULT_CONFIG_PATH"

#if defined(__linux__)

#include <linux/limits.h>

extern char env_config_dir_path[PATH_MAX];
extern char env_data_dir_path[PATH_MAX];
extern char env_sercret_dir_path[PATH_MAX];

extern char db_config_path[PATH_MAX];
extern char db_vault_path[PATH_MAX];
extern char titan_key_path[PATH_MAX];

#else
// TODO: add compatibility with other systems
#endif

/**
 * Length of the string stored in env_config_dir_path
 */
extern size_t env_cdp_len;

/**
 * Length of the string stored in env_data_dir_path
 */
extern size_t env_ddp_len;

/**
 * Length of the string stored in env_sercret_dir_path
 */
extern size_t env_sdp_len;

/**
 * Enum defining the status codes for environment operations.
 * These codes indicate the success or specific failure reason of environment
 * initialization.
 */
typedef enum {
    ENV_SUCCESS = 0,
    ENV_SYSCALL_ERR,
    ENV_NO_DIR,
    ENV_NO_HOME,
    ENV_REPO_ERR,
    ENV_PER_ERR,
    ENV_NOT_SUPPORTED
} env_status_enum;

/**
 * @brief Stores the status code of the last environment operation.
 * Use this variable to check for errors after calling environment functions.
 */
extern env_status_enum status;

/**
 * @brief Initializes the environment paths (configuration, data, secrets).
 * This function determines and sets the necessary directory paths based on the
 * operating system and user environment.
 *
 * @return `true` if the paths were initialized successfully, `false` otherwise.
 */
bool initialize_paths();

bool get_paths();

/**
 * @brief Checks if the application environment has been initialized.
 *
 * @return `true` if the environment is already initialized, `false` otherwise.
 */
bool environment_is_init();

/**
 * @brief Performs the complete initialization of the application environment.
 * This includes setting up all necessary paths and ensuring the environment is
 * ready for operation.
 *
 * @return `true` if the environment was successfully initialized, `false`
 * otherwise.
 */
bool initialize_environment();

#endif

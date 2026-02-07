/**
 * @file config_keys.h
 * @brief Defines string constants for CVault configuration keys.
 *
 * This file contains the standard key names used in the key-value configuration
 * store (SQLite table or config file). Using these constants prevents magic strings
 * and ensures consistency across the application.
 */
#ifndef CONFIG_KEYS_H
#define CONFIG_KEYS_H

#define CFG_APP_VER "version"
#define CFG_SCHEMA_VER "schema_version"
#define CFG_SALT "salt"
#define CFG_VER_KEY "verification_key"
#define CFG_ITERATIONS "kdf_iterations"
#define CFG_MEMORY_COST "kdf_memory"
#define CFG_PARALLELISM "kdf_parallelism"
#define CFG_VAULT_PATH "vault_path"
#define CFG_TITAN_KEY_PATH "titan_key_path"
#define CFG_BACKUP_PATH "backup_path"
#define CFG_AUTO_LOCK_TMOUT "auto_lock_timeout"
#define CFG_CLIP_CLR_TMOUT "clipboard_clear"
#define CFG_LANG "language"
#define CFG_SHOW_STRENGTH "show_strengh_meters"
#define CFG_PSSWRD_GEN_CHARSET "password_generating_charset"
#define CFG_PSSWRD_MASK "mask_password"

#endif

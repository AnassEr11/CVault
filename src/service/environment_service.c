#include <CVault/models/config.h>
#include <CVault/models/config_keys.h>
#include <CVault/repository/repository.h>
#include <CVault/service/environment_service.h>

#include <CVault/utils/security_utils.h>

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vendor/sqlite3/sqlite3.h>

#if defined(__linux__)

#include <errno.h>
#include <fcntl.h>
#include <linux/limits.h>
#include <sys/stat.h>

char env_config_dir_path[PATH_MAX] = "";
char env_data_dir_path[PATH_MAX] = "";
char env_sercret_dir_path[PATH_MAX] = "";

char db_config_path[PATH_MAX] = "";
char db_vault_path[PATH_MAX] = "";
char titan_key_path[PATH_MAX] = "";

#endif

size_t env_cdp_len = 0;
size_t env_ddp_len = 0;
size_t env_sdp_len = 0;

env_status_enum status = 0;

bool initialize_paths() {
#if defined(__linux__)

    char *home_dir = getenv("HOME");
    if (!home_dir) {
        status = ENV_NO_HOME;
        return false;
    }

    char *config_home;
    snprintf(env_config_dir_path, PATH_MAX, "%s/%s", home_dir, CONFIG_HOME_PATH);

    char *data_home;
    if (!(data_home = getenv("XDG_DATA_HOME"))) {
        snprintf(env_data_dir_path, PATH_MAX, "%s/.local/share/cvault", home_dir);
    } else {
        snprintf(env_data_dir_path, PATH_MAX, "%s/cvault", data_home);
    }

    snprintf(env_sercret_dir_path, PATH_MAX, "%s/secrets", env_data_dir_path);

    snprintf(db_config_path, PATH_MAX, "%s/%s", env_config_dir_path, DB_CONFIG_FILE);
    snprintf(db_vault_path, PATH_MAX, "%s/%s", env_data_dir_path, DB_VAULT_FILE);
    snprintf(titan_key_path, PATH_MAX, "%s/%s", env_sercret_dir_path, TITAN_KEY_FILE);

    Config *config = malloc(sizeof(Config));

    config->config_key = CFG_VAULT_PATH;
    config->config_value = (uint8_t *)strdup(db_vault_path);
    config->config_value_len = strlen(db_vault_path);

    sqlite3 *db;
    if (sqlite3_open(db_config_path, &db) != SQLITE_OK) {
        status = ENV_REPO_ERR;
        return false;
    }

    if (add_config(config, db) != OK) {
        status = ENV_REPO_ERR;
        return false;
    }

    free(config->config_value);
    free(config);

    config = malloc(sizeof(Config));

    config->config_key = CFG_TITAN_KEY_PATH;
    config->config_value = (uint8_t *)strdup(titan_key_path);
    config->config_value_len = strlen(titan_key_path);

    if (add_config(config, db) != OK) {
        status = ENV_REPO_ERR;
        return false;
    }

    free(config->config_value);
    free(config);

    sqlite3_close(db);
#else
    // TODO: add portability to other platforms
    status = ENV_NOT_SUPPORTED;
    return false;
#endif /* if defined (__linux__) */

    env_cdp_len = strlen(env_config_dir_path);
    env_ddp_len = strlen(env_data_dir_path);
    env_sdp_len = strlen(env_sercret_dir_path);

    return true;
}

bool get_paths() {
#if defined(__linux__)
    secure_memset(db_config_path,PATH_MAX);
    secure_memset(db_vault_path,PATH_MAX);
    secure_memset(titan_key_path,PATH_MAX);

    char *home_path = getenv("HOME");
    if (!home_path) {
        status = ENV_NO_HOME;
        return false;
    }

    snprintf(db_config_path, PATH_MAX, "%s/%s/%s", home_path, CONFIG_HOME_PATH,DB_CONFIG_FILE);

    sqlite3 *db;
    if (sqlite3_open(db_config_path, &db) != SQLITE_OK) {
        status = ENV_REPO_ERR;
        return false;
    }

    Config *out_config = malloc(sizeof(Config));

    if (read_config(CFG_VAULT_PATH, out_config, db) != OK) {
        status = ENV_REPO_ERR;
        return false;
    }
    snprintf(db_vault_path,PATH_MAX,"%s", (char *)out_config->config_value);
    db_vault_path[out_config->config_value_len] = '\0';

    free(out_config->config_key);
    free(out_config->config_value);
    free(out_config);

    out_config = malloc(sizeof(Config));
    if (read_config(CFG_TITAN_KEY_PATH, out_config, db) != OK) {
        status = ENV_REPO_ERR;
        return false;
    }
    strncpy(titan_key_path, (char *)out_config->config_value, PATH_MAX);
    titan_key_path[out_config->config_value_len] = '\0';

    free(out_config->config_key);
    free(out_config->config_value);
    free(out_config);

    sqlite3_close(db);
#endif /* if defined (__linux__) */

    return true;
}

bool environment_is_init() {
    if (!env_cdp_len || !env_ddp_len || !env_sdp_len) {
        return false;
    }

#if defined(__linux__)

    struct stat st;

    if (stat(env_config_dir_path, &st) == -1) {
        status = ENV_NO_DIR;
        return false;
    } else {
        if (!(st.st_mode & 0700)) {
            status = ENV_PER_ERR;
            return false;
        }
    }

    if (stat(env_data_dir_path, &st) == -1) {
        status = ENV_NO_DIR;
        return false;
    } else {
        if (!(st.st_mode & 0700)) {
            status = ENV_PER_ERR;
            return false;
        }
    }

    if (stat(env_sercret_dir_path, &st) == -1) {
        status = ENV_NO_DIR;
        return false;
    } else {
        if (!(st.st_mode & 0700)) {
            status = ENV_PER_ERR;
            return false;
        }
    }

#else
    // TODO: add portability to other platforms
    status = ENV_NOT_SUPPORTED;
    return false
#endif /* if defined (__linux__) */

    return true;
}

static bool create_path(char *path, size_t length) {
#if defined(__linux__)

    int c = 0;
    for (size_t i = 1; i < length; i++) {
        if (path[i] == '/') {
            c++;
            if (c > 2) {
                path[i] = '\0';

                if (mkdir(path, 0700) == -1) {
                    if (errno != EEXIST) {
                        status = ENV_SYSCALL_ERR;
                        return false;
                    }
                }

                path[i] = '/';
            }
        }
    }

    /*
     * the following system call create the deepest directory
     * as it's not followed by a '/'
     */
    if (mkdir(path, 0700) == -1) {
        if (errno != EEXIST) {
            status = ENV_SYSCALL_ERR;
            return false;
        }
    }

#else
    // TODO: add portability to other platforms
    status = ENV_NOT_SUPPORTED;
    return false;
#endif /* if defined (__linux__) */

    return true;
}

bool initialize_environment() {
    if (!initialize_paths()) {
        return false;
    }

    if (!create_path(env_config_dir_path, env_cdp_len)) {
        return false;
    }

    if (!create_path(env_data_dir_path, env_ddp_len)) {
        return false;
    }

    if (!create_path(env_sercret_dir_path, env_sdp_len)) {
        return false;
    }

    return true;
}

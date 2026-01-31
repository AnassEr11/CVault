// TODO: modify this logic to store paths in the database

#include <CVault/service/environment_service.h>
#include <linux/limits.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(__linux__)

#include <errno.h>
#include <fcntl.h>
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
    if (!(config_home = getenv("XDG_CONFIG_HOME"))) {
        snprintf(env_config_dir_path, PATH_MAX, "%s/.config/cvault", home_dir);
    } else {
        snprintf(env_config_dir_path, PATH_MAX, "%s/cvault", config_home);
    }

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

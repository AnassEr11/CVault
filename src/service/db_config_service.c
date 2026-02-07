#include <CVault/service/db_config_service.h>
#include <CVault/service/environment_service.h>
#include <CVault/utils/security_utils.h>
#include <stdlib.h>
#include <string.h>

static sqlite3 *db = NULL;

bool open_config_service() {
    if (!initialize_paths()) {
        return false;
    }

    if (sqlite3_open(db_config_path, &db) != SQLITE_OK) {
        return false;
    }

    return db != NULL;
}

bool service_add_config(Config *config) {
    if (!config || !config->config_key || !config->config_value || !config->config_value_len) {
        return false;
    }

    if (add_config(config, db) != OK) {
        return false;
    }

    return true;
}

bool service_read_config(char *key, Config *out_config) {
    if (!key || !out_config) {
        return false;
    }

    if (read_config(key, out_config, db) != OK) {
        return false;
    }

    if (!out_config->config_value || !out_config->config_key || !out_config->config_value_len) {
        return false;
    }

    return true;
}

bool service_update_config(char *key, uint8_t *new_value, size_t new_size) {
    if (!key || !new_value || !new_size) {
        return false;
    }

    Config buffer = {.config_key = key, .config_value = new_value, .config_value_len = new_size};

    if (update_config(key, &buffer, db) != SQLITE_OK) {
        return false;
    }

    Config *check_buffer = malloc(sizeof(Config));

    bool return_code = true;

    if (read_config(key, check_buffer, db) != OK) {
        return_code = false;
        goto finish;
    }

    if (new_size != check_buffer->config_value_len) {
        return_code = false;
        goto finish;
    }

    if (!constant_time_equal(new_value, check_buffer->config_value, new_size)) {
        return_code = false;
        goto finish;
    }

finish:
    free(check_buffer->config_key);
    free(check_buffer->config_value);
    return return_code;
}

bool service_delete_config(char *key) {
    if (!key) {
        return false;
    }

    if (delete_configs(key, db) != OK) {
        return false;
    }

    Config *check_buffer = malloc(sizeof(Config));

    repo_return_code rc = read_config(key, check_buffer, db);

    if (rc == NOT_FOUND_ERR) {
        return true;
    }

    if (rc == OK) {
        free(check_buffer->config_key);
        free(check_buffer->config_value);
        return false;
    }

    return false;
}

bool service_delete_all_configs() {
    if (delete_all_configs(db) == OK) {
        return true;
    }

    return false;
}

bool close_config_service() {
    return sqlite3_close(db) == SQLITE_OK;
}

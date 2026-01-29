#include <CVault/repository/repository.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

repo_return_code add_config(Config *config, sqlite3 *db) {
    char *sql_query = "INSERT INTO configs (config_key, config_value)"
                      "VALUES (?,?)";

    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, sql_query, -1, &stmt, NULL) != SQLITE_OK) {
        return DATA_BASE_ERR;
    }

    if (sqlite3_bind_text(stmt, 1, config->config_key, -1, SQLITE_TRANSIENT) != SQLITE_OK) {
        sqlite3_finalize(stmt);
        return DATA_BASE_ERR;
    }

    if (sqlite3_bind_blob(stmt, 2, config->config_value, config->config_value_len,
                          SQLITE_TRANSIENT) != SQLITE_OK) {
        sqlite3_finalize(stmt);
        return DATA_BASE_ERR;
    }

    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    switch (rc) {
        case SQLITE_DONE:
            return OK;

        case SQLITE_ERROR:
            return DATA_BASE_ERR;

        default:
            return REPO_UNEXPECTED_ERR;
    }
}
repo_return_code read_config(const char *key, Config *out_config, sqlite3 *db) {
    char *sql_query = "SELECT * FROM configs WHERE config_key = ?";

    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, sql_query, -1, &stmt, NULL) != SQLITE_OK) {
        return DATA_BASE_ERR;
    }

    if (sqlite3_bind_text(stmt, 1, key, -1, SQLITE_TRANSIENT) != SQLITE_OK) {
        sqlite3_finalize(stmt);
        return DATA_BASE_ERR;
    }

    int rc = sqlite3_step(stmt);

    switch (rc) {
        case SQLITE_ERROR:
            sqlite3_finalize(stmt);
            return DATA_BASE_ERR;

        case SQLITE_DONE:
            sqlite3_finalize(stmt);
            return NOT_FOUND_ERR;

        case SQLITE_ROW:
            char *tmp_key = (char *)sqlite3_column_text(stmt, 0);
            if (!(out_config->config_key = strdup(tmp_key))) {
                sqlite3_finalize(stmt);
                return MEMORY_ERR;
            }

            out_config->config_value_len = sqlite3_column_bytes(stmt, 1);

            uint8_t *tmp_config_value = (uint8_t *)sqlite3_column_blob(stmt, 1);
            out_config->config_value = malloc(out_config->config_value_len);
            if (!out_config->config_value) {
                sqlite3_finalize(stmt);
                return MEMORY_ERR;
            }
            memcpy(out_config->config_value, tmp_config_value, out_config->config_value_len);

            sqlite3_finalize(stmt);
            return OK;

        default:
            return REPO_UNEXPECTED_ERR;
    }
}
repo_return_code update_config(const char *key, Config *new_config, sqlite3 *db) {
    char *sql_query = "UPDATE configs SET "
                      "config_value = ? "
                      "WHERE config_key = ?";

    sqlite3_stmt *stmt;

    if (sqlite3_prepare_v2(db, sql_query, -1, &stmt, NULL) != SQLITE_OK) {
        return DATA_BASE_ERR;
    }

    if (sqlite3_bind_blob(stmt, 1, new_config->config_value, new_config->config_value_len,
                          SQLITE_TRANSIENT) != SQLITE_OK) {
        sqlite3_finalize(stmt);
        return DATA_BASE_ERR;
    }

    if (sqlite3_bind_text(stmt, 2, new_config->config_key, -1, SQLITE_TRANSIENT) != SQLITE_OK) {
        sqlite3_finalize(stmt);
        return DATA_BASE_ERR;
    }

    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    switch (rc) {
        case SQLITE_DONE:
            return (sqlite3_changes(db)) ? OK : NOT_FOUND_ERR;

        case SQLITE_ERROR:
            return DATA_BASE_ERR;

        default:
            return REPO_UNEXPECTED_ERR;
    }
}
repo_return_code delete_configs(const char *key, sqlite3 *db) {
    char *sql_query = "DELETE FROM configs WHERE config_key = ?";
    sqlite3_stmt *stmt;

    if (sqlite3_prepare_v2(db, sql_query, -1, &stmt, NULL) != SQLITE_OK) {
        return DATA_BASE_ERR;
    }

    if (sqlite3_bind_text(stmt, 1, key, -1, SQLITE_TRANSIENT) != SQLITE_OK) {
        sqlite3_finalize(stmt);
        return DATA_BASE_ERR;
    }

    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    switch (rc) {
        case SQLITE_DONE:
            return (sqlite3_changes(db)) ? OK : NOT_FOUND_ERR;

        case SQLITE_ERROR:
            return DATA_BASE_ERR;

        default:
            return REPO_UNEXPECTED_ERR;
    }
}
repo_return_code delete_all_configs(sqlite3 *db) {
    char *sql_query = "DELETE FROM configs";
    char *err_msg = 0;

    if (sqlite3_exec(db, sql_query, NULL, 0, &err_msg) != SQLITE_OK) {
        sqlite3_free(err_msg);
        return DATA_BASE_ERR;
    }

    return OK;
}

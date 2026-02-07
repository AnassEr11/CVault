#include <CVault/repository/repository.h>
#include <CVault/service/db_init_service.h>
#include <CVault/service/environment_service.h>
#include <vendor/sqlite3/sqlite3.h>

#if defined(__linux__)

#include <unistd.h>

#endif /* if defined (__linux__) */

static bool open_db_file(char *path, sqlite3 **db);
static bool close_db_file(sqlite3 *db);
static bool table_exists(sqlite3 *db, const char *table_name);

bool init_schema() {
    sqlite3 *config_db = NULL;
    sqlite3 *vault_db = NULL;
    if (!initialize_paths()) {
        return false;
    }

    if (!open_db_file(db_config_path, &config_db)) {
        return false;
    }

    if (!open_db_file(db_vault_path, &vault_db)) {
        return false;
    }

    if (repo_config_init(config_db) != OK) {
        return false;
    }

    if (repo_vault_init(vault_db) != OK) {
        return false;
    }

    if (!close_db_file(config_db)) {
        return false;
    }

    if (!close_db_file(vault_db)) {
        return false;
    }

    return true;
}

bool is_init_schema() {

#if defined(__linux__)
    if (access(db_config_path, F_OK) != 0 || access(db_vault_path, F_OK) != 0) {
        return false;
    }
#else
    return false;
#endif /* if defined (__linux__) */

    sqlite3 *temp_config = NULL;
    sqlite3 *temp_vault = NULL;
    bool valid = true;

    if (!open_db_file(db_config_path, &temp_config)) {
        valid = false;
    }
    if (valid && !open_db_file(db_vault_path, &temp_vault)) {
        valid = false;
    }

    if (valid) {
        if (!table_exists(temp_vault, "entries")) {
            valid = false;
        }
        if (!table_exists(temp_config, "configs")) {
            valid = false;
        }
    }

    if (temp_config) {
        close_db_file(temp_config);
    }

    if (temp_vault) {
        close_db_file(temp_vault);
    }

    return valid;
}

static bool open_db_file(char *path, sqlite3 **db) {
    if (!path) {
        return false;
    }

    if (sqlite3_open(path, db) != SQLITE_OK) {
        return false;
    }
    return *db != NULL;
}

static bool close_db_file(sqlite3 *db) {
    if (sqlite3_close(db)) {
        return false;
    }
    return true;
}

static bool table_exists(sqlite3 *db, const char *table_name) {
    sqlite3_stmt *stmt;
    const char *query = "SELECT count(*) FROM sqlite_master WHERE type='table' AND name=?";

    if (sqlite3_prepare_v2(db, query, -1, &stmt, NULL) != SQLITE_OK) {
        return false;
    }

    sqlite3_bind_text(stmt, 1, table_name, -1, SQLITE_STATIC);

    bool exists = false;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        exists = sqlite3_column_int(stmt, 0) > 0;
    }

    sqlite3_finalize(stmt);
    return exists;
}

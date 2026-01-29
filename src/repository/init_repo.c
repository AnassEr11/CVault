#include <CVault/repository/repository.h>
#include <stdlib.h>

repo_return_code repo_init(sqlite3 *db) {
    char *sql_create_entries_table = "CREATE TABLE IF NOT EXISTS entries ("
                                     "uuid CHAR(36) PRIMARY KEY NOT NULL,"
                                     "service_blob BLOB NOT NULL,"
                                     "username_blob BLOB NOT NULL,"
                                     "password_blob BLOB NOT NULL,"
                                     "notes_blob BLOB,"
                                     "created_at INTEGER NOT NULL,"
                                     "updated_at INTEGER NOT NULL"
                                     ");";
    if (sqlite3_exec(db, sql_create_entries_table, NULL, NULL, NULL) != SQLITE_OK) {
        return DATA_BASE_ERR;
    }

    char *sql_create_configs_table = "CREATE TABLE IF NOT EXISTS configs ("
                                     "config_key TEXT PRIMARY KEY NOT NULL,"
                                     "config_value BLOB NOT NULL"
                                     ");";
    if (sqlite3_exec(db, sql_create_configs_table, NULL, NULL, NULL) != SQLITE_OK) {
        return DATA_BASE_ERR;
    }

    if (sqlite3_exec(db, "PRAGMA journal_mode=WAL;", NULL, NULL, NULL) != SQLITE_OK) {
        return DATA_BASE_ERR;
    }

    return OK;
}

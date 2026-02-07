#include "CVault/utils/data_structure_utils.h"
#include "sqlite3/sqlite3.h"
#include <CVault/models/vault_entry.h>
#include <CVault/repository/repository.h>
#include <CVault/utils/security_utils.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

repo_return_code repo_vault_init(sqlite3 *db) {
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

    if (sqlite3_exec(db, "PRAGMA journal_mode=WAL;", NULL, NULL, NULL) != SQLITE_OK) {
        return DATA_BASE_ERR;
    }

    return OK;
}

repo_return_code add_entry(IntVaultEntry *entry, sqlite3 *db) {
    char *sql_query =
        "INSERT INTO entries "
        "(uuid, service_blob, username_blob, password_blob, notes_blob, created_at, updated_at) "
        "VALUES (?, ?, ?, ?, ?, ?, ?)";

    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, sql_query, -1, &stmt, NULL) != SQLITE_OK) {
        return DATA_BASE_ERR;
    }

    if (sqlite3_bind_text(stmt, 1, (const char *)entry->uuid, -1, SQLITE_TRANSIENT) != SQLITE_OK) {
        sqlite3_finalize(stmt);
        return DATA_BASE_ERR;
    }
    if (sqlite3_bind_blob(stmt, 2, (const void *)entry->service_name, (int)entry->service_len,
                          SQLITE_TRANSIENT) != SQLITE_OK) {
        sqlite3_finalize(stmt);
        return DATA_BASE_ERR;
    }
    if (sqlite3_bind_blob(stmt, 3, (const void *)entry->username, (int)entry->username_len,
                          SQLITE_TRANSIENT) != SQLITE_OK) {
        sqlite3_finalize(stmt);
        return DATA_BASE_ERR;
    }
    if (sqlite3_bind_blob(stmt, 4, (const void *)entry->password, (int)entry->password_len,
                          SQLITE_TRANSIENT) != SQLITE_OK) {
        sqlite3_finalize(stmt);
        return DATA_BASE_ERR;
    }
    if (entry->notes != NULL) {
        if (sqlite3_bind_blob(stmt, 5, (const void *)entry->notes, (int)entry->notes_len,
                              SQLITE_TRANSIENT) != SQLITE_OK) {
            sqlite3_finalize(stmt);
            return DATA_BASE_ERR;
        }
    } else {
        if (sqlite3_bind_null(stmt, 5) != SQLITE_OK) {
            return DATA_BASE_ERR;
            sqlite3_finalize(stmt);
        }
    }
    if (sqlite3_bind_int(stmt, 6, (int)entry->created_at) != SQLITE_OK) {
        return DATA_BASE_ERR;
        sqlite3_finalize(stmt);
    }
    if (sqlite3_bind_int(stmt, 7, (int)entry->updated_at) != SQLITE_OK) {
        sqlite3_finalize(stmt);
        return DATA_BASE_ERR;
    }

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        sqlite3_finalize(stmt);
        return DATA_BASE_ERR;
    }

    sqlite3_finalize(stmt);
    return OK;
}

repo_return_code read_entry(const char *uuid, IntVaultEntry *out_entry, sqlite3 *db) {
    char *sql_query = "SELECT * FROM entries WHERE uuid = ?";

    sqlite3_stmt *stmt;

    if (sqlite3_prepare_v2(db, sql_query, -1, &stmt, NULL) != SQLITE_OK) {
        return DATA_BASE_ERR;
    }

    if (sqlite3_bind_text(stmt, 1, uuid, -1, SQLITE_TRANSIENT) != SQLITE_OK) {
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
            char *tmp_uuid = (char *)sqlite3_column_text(stmt, 0);
            if (!(out_entry->uuid = strdup(tmp_uuid))) {
                sqlite3_finalize(stmt);
                return MEMORY_ERR;
            }

            uint8_t *tmp_service = (uint8_t *)sqlite3_column_blob(stmt, 1);
            out_entry->service_name =
                (uint8_t *)malloc(out_entry->service_len = sqlite3_column_bytes(stmt, 1));
            if (!out_entry->service_name) {
                sqlite3_finalize(stmt);
                return MEMORY_ERR;
            }
            memcpy(out_entry->service_name, tmp_service, out_entry->service_len);

            uint8_t *tmp_username = (uint8_t *)sqlite3_column_blob(stmt, 2);
            out_entry->username =
                (uint8_t *)malloc(out_entry->username_len = sqlite3_column_bytes(stmt, 2));
            if (!out_entry->username) {
                sqlite3_finalize(stmt);
                return MEMORY_ERR;
            }
            memcpy(out_entry->username, tmp_username, out_entry->username_len);

            uint8_t *tmp_password = (uint8_t *)sqlite3_column_blob(stmt, 3);
            out_entry->password =
                (uint8_t *)malloc(out_entry->password_len = sqlite3_column_bytes(stmt, 3));
            if (!out_entry->password) {
                sqlite3_finalize(stmt);
                return MEMORY_ERR;
            }
            memcpy(out_entry->password, tmp_password, out_entry->password_len);

            if ((out_entry->notes_len = sqlite3_column_bytes(stmt, 4))) {
                uint8_t *tmp_notes = (uint8_t *)sqlite3_column_blob(stmt, 4);
                out_entry->notes = (uint8_t *)malloc(out_entry->password_len);
                if (!out_entry->notes) {
                    sqlite3_finalize(stmt);
                    return MEMORY_ERR;
                }
                memcpy(out_entry->notes, tmp_notes, out_entry->notes_len);
            } else {
                out_entry->notes = NULL;
            }

            out_entry->created_at = sqlite3_column_int64(stmt, 5);

            out_entry->updated_at = sqlite3_column_int64(stmt, 6);

            sqlite3_finalize(stmt);
            return OK;
        default:
            sqlite3_finalize(stmt);
            return REPO_UNEXPECTED_ERR;
    }
}
repo_return_code read_all_entries(DLinkedList *out_wrapper, sqlite3 *db) {
    char *sql_query = "SELECT * FROM entries";
    sqlite3_stmt *stmt;

    if ((sqlite3_prepare_v2(db, sql_query, -1, &stmt, NULL)) != SQLITE_OK) {
        return DATA_BASE_ERR;
    }

    int rc;
    IntVaultEntry *buffer;
    while (true) {
        rc = sqlite3_step(stmt);
        switch (rc) {
            case SQLITE_ERROR:
                sqlite3_finalize(stmt);
                return DATA_BASE_ERR;
            case SQLITE_DONE:
                sqlite3_finalize(stmt);
                return OK;
            case SQLITE_ROW:
                buffer = malloc(sizeof(IntVaultEntry));

                char *tmp_uuid = (char *)sqlite3_column_text(stmt, 0);
                if (!(buffer->uuid = strdup(tmp_uuid))) {
                    sqlite3_finalize(stmt);
                    return MEMORY_ERR;
                }

                uint8_t *tmp_service = (uint8_t *)sqlite3_column_blob(stmt, 1);
                buffer->service_name =
                    (uint8_t *)malloc(buffer->service_len = sqlite3_column_bytes(stmt, 1));
                if (!buffer->service_name) {
                    sqlite3_finalize(stmt);
                    return MEMORY_ERR;
                }
                memcpy(buffer->service_name, tmp_service, buffer->service_len);

                uint8_t *tmp_username = (uint8_t *)sqlite3_column_blob(stmt, 2);
                buffer->username =
                    (uint8_t *)malloc(buffer->username_len = sqlite3_column_bytes(stmt, 2));
                if (!buffer->username) {
                    sqlite3_finalize(stmt);
                    return MEMORY_ERR;
                }
                memcpy(buffer->username, tmp_username, buffer->username_len);

                uint8_t *tmp_password = (uint8_t *)sqlite3_column_blob(stmt, 3);
                buffer->password =
                    (uint8_t *)malloc(buffer->password_len = sqlite3_column_bytes(stmt, 3));
                if (!buffer->password) {
                    sqlite3_finalize(stmt);
                    return MEMORY_ERR;
                }
                memcpy(buffer->password, tmp_password, buffer->password_len);

                if ((buffer->notes_len = sqlite3_column_bytes(stmt, 4))) {
                    uint8_t *tmp_notes = (uint8_t *)sqlite3_column_blob(stmt, 4);
                    buffer->notes = (uint8_t *)malloc(buffer->notes_len);
                    if (!buffer->notes) {
                        sqlite3_finalize(stmt);
                        return MEMORY_ERR;
                    }
                    memcpy(buffer->notes, tmp_notes, buffer->notes_len);
                } else {
                    buffer->notes = NULL;
                }

                buffer->created_at = sqlite3_column_int64(stmt, 5);

                buffer->updated_at = sqlite3_column_int64(stmt, 6);

                void *tmp = dlinked_list_push_back_node(out_wrapper, buffer);
                if (tmp == NULL) {
                    sqlite3_finalize(stmt);
                    return DATA_STRUCTURE_ERR;
                } else {
                    out_wrapper = tmp;
                }
                break;
            default:
                sqlite3_finalize(stmt);
                return REPO_UNEXPECTED_ERR;
        }
    }
    sqlite3_finalize(stmt);
    return OK;
}
repo_return_code update_entry(const char *uuid, IntVaultEntry *new_entry, sqlite3 *db) {
    char *sql_query = "UPDATE entries SET "
                      "service_blob = COALESCE(?, service_blob), "
                      "username_blob = COALESCE(?, username_blob), "
                      "password_blob = COALESCE(?, password_blob), "
                      "notes_blob = COALESCE(?,notes_blob), "
                      "updated_at = ? "
                      "WHERE uuid = ?";

    sqlite3_stmt *stmt;

    if (sqlite3_prepare_v2(db, sql_query, -1, &stmt, NULL) != SQLITE_OK) {
        return DATA_BASE_ERR;
    }

    if (new_entry->service_name == NULL) {
        if (sqlite3_bind_null(stmt, 1) != SQLITE_OK) {
            sqlite3_finalize(stmt);
            return DATA_BASE_ERR;
        }
    } else {
        if (sqlite3_bind_blob(stmt, 1, new_entry->service_name, new_entry->service_len,
                              SQLITE_TRANSIENT) != SQLITE_OK) {
            sqlite3_finalize(stmt);
            return DATA_BASE_ERR;
        }
    }

    if (new_entry->username == NULL) {
        if (sqlite3_bind_null(stmt, 2) != SQLITE_OK) {
            sqlite3_finalize(stmt);
            return DATA_BASE_ERR;
        }
    } else {
        if (sqlite3_bind_blob(stmt, 2, new_entry->username, new_entry->username_len,
                              SQLITE_TRANSIENT) != SQLITE_OK) {
            sqlite3_finalize(stmt);
            return DATA_BASE_ERR;
        }
    }

    if (new_entry->password == NULL) {
        if (sqlite3_bind_null(stmt, 3) != SQLITE_OK) {
            sqlite3_finalize(stmt);
            return DATA_BASE_ERR;
        }
    } else {
        if (sqlite3_bind_blob(stmt, 3, new_entry->password, new_entry->password_len,
                              SQLITE_TRANSIENT) != SQLITE_OK) {
            sqlite3_finalize(stmt);
            return DATA_BASE_ERR;
        }
    }

    if (new_entry->notes == NULL) {
        if (sqlite3_bind_null(stmt, 4) != SQLITE_OK) {
            sqlite3_finalize(stmt);
            return DATA_BASE_ERR;
        }
    } else {
        if (sqlite3_bind_blob(stmt, 4, new_entry->notes, new_entry->notes_len, SQLITE_TRANSIENT) !=
            SQLITE_OK) {
            sqlite3_finalize(stmt);
            return DATA_BASE_ERR;
        }
    }

    if (sqlite3_bind_int(stmt, 5, new_entry->updated_at) != SQLITE_OK) {
        sqlite3_finalize(stmt);
        return DATA_BASE_ERR;
    }

    if (sqlite3_bind_text(stmt, 6, uuid, -1, SQLITE_TRANSIENT) != SQLITE_OK) {
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
repo_return_code delete_entry(const char *uuid, sqlite3 *db) {
    char *sql_query = "DELETE FROM entries WHERE uuid = ?";
    sqlite3_stmt *stmt;

    if (sqlite3_prepare_v2(db, sql_query, -1, &stmt, NULL) != SQLITE_OK) {
        return DATA_BASE_ERR;
    }

    if (sqlite3_bind_text(stmt, 1, uuid, -1, SQLITE_TRANSIENT) != SQLITE_OK) {
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
repo_return_code delete_all_entries(sqlite3 *db) {
    char *sql_query = "DELETE FROM entries";
    char *err_msg = 0;

    if (sqlite3_exec(db, sql_query, NULL, 0, &err_msg) != SQLITE_OK) {
        sqlite3_free(err_msg);
        return DATA_BASE_ERR;
    }

    return OK;
}

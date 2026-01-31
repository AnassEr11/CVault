#include <CVault/repository/repository.h>
#include <CVault/utils/data_structure_utils.h>
#include <CVault/utils/security_utils.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#if defined(__linux__)
#include <unistd.h>
#endif

#define COLOR_RESET "\033[0m"
#define COLOR_GREEN "\033[0;32m"
#define COLOR_RED "\033[0;31m"
#define COLOR_BLUE "\033[34m"
#define COLOR_YELLOW "\033[1;33m"
#define COLOR_CYAN "\033[0;36m"

#define DB_PATH "/tmp/cvault_test.db"

sqlite3 *db = NULL;
IntVaultEntry *entry1;
IntVaultEntry *entry2;
repo_return_code op_status;

bool clean_environment();
bool init_test();
bool test_repo_init();
bool test_add_entry();
bool test_read_entry();
bool test_read_all_entries();
bool test_update_entry();
bool test_delete_entry();
bool test_delete_all_entries();

bool close_test();

int main() {
    printf("\n%sTEST ENTRIES REPOSITORY OPERATIONS%s\n\n", COLOR_BLUE, COLOR_RESET);

    printf("%s[TEST 1/8]%s Initializing database...\n", COLOR_BLUE, COLOR_RESET);
    if (!init_test()) {
        printf("%s[FAILED]%s Database initialization failed\n\n", COLOR_RED, COLOR_RESET);
        return 1;
    }
    printf("%s[PASSED]%s Database initialized successfully\n\n", COLOR_GREEN, COLOR_RESET);

    printf("%s[TEST 2/8]%s Initializing repository...\n", COLOR_BLUE, COLOR_RESET);
    if (!test_repo_init()) {
        printf("%s[FAILED]%s Repository initialization failed\n\n", COLOR_RED, COLOR_RESET);
        sqlite3_close(db);
        return 1;
    }
    printf("%s[PASSED]%s Repository initialized successfully\n\n", COLOR_GREEN, COLOR_RESET);

    printf("%s[TEST 3/8]%s Adding entries...\n", COLOR_BLUE, COLOR_RESET);
    if (!test_add_entry()) {
        printf("%s[FAILED]%s Failed to add entries\n\n", COLOR_RED, COLOR_RESET);
        sqlite3_close(db);
        return 1;
    }
    printf("%s[PASSED]%s Entries added successfully\n\n", COLOR_GREEN, COLOR_RESET);

    printf("%s[TEST 4/8]%s Reading single entry...\n", COLOR_BLUE, COLOR_RESET);
    if (!test_read_entry()) {
        printf("%s[FAILED]%s Failed to read entry\n\n", COLOR_RED, COLOR_RESET);
        sqlite3_close(db);
        return 1;
    }
    printf("%s[PASSED]%s Entry read successfully\n\n", COLOR_GREEN, COLOR_RESET);

    printf("%s[TEST 5/8]%s Reading all entries...\n", COLOR_BLUE, COLOR_RESET);
    if (!test_read_all_entries()) {
        printf("%s[FAILED]%s Failed to read all entries\n\n", COLOR_RED, COLOR_RESET);
        sqlite3_close(db);
        return 1;
    }
    printf("%s[PASSED]%s All entries read successfully\n\n", COLOR_GREEN, COLOR_RESET);

    printf("%s[TEST 6/8]%s Updating entry...\n", COLOR_BLUE, COLOR_RESET);
    if (!test_update_entry()) {
        printf("%s[FAILED]%s Failed to update entry\n\n", COLOR_RED, COLOR_RESET);
        sqlite3_close(db);
        return 1;
    }
    printf("%s[PASSED]%s Entry updated successfully\n\n", COLOR_GREEN, COLOR_RESET);

    printf("%s[TEST 7/8]%s Deleting entry...\n", COLOR_BLUE, COLOR_RESET);
    if (!test_delete_entry()) {
        printf("%s[FAILED]%s Failed to delete entry\n\n", COLOR_RED, COLOR_RESET);
        sqlite3_close(db);
        return 1;
    }
    printf("%s[PASSED]%s Entry deleted successfully\n\n", COLOR_GREEN, COLOR_RESET);

    printf("%s[TEST 8/8]%s Deleting all entries...\n", COLOR_BLUE, COLOR_RESET);
    if (!test_delete_all_entries()) {
        printf("%s[FAILED]%s Failed to delete all entries\n\n", COLOR_RED, COLOR_RESET);
        sqlite3_close(db);
        return 1;
    }
    printf("%s[PASSED]%s All entries deleted successfully\n\n", COLOR_GREEN, COLOR_RESET);

    printf("%s[CLEANUP]%s Closing database...\n", COLOR_YELLOW, COLOR_RESET);
    if (!close_test()) {
        printf("%s[WARNING]%s Database close failed\n\n", COLOR_YELLOW, COLOR_RESET);
    } else {
        printf("%s[CLEANUP DONE]%s Database closed successfully\n\n", COLOR_GREEN, COLOR_RESET);
    }

    printf("%sENTRIES REPO OPERATIONS TEST COMPLETED%s\n", COLOR_BLUE, COLOR_RESET);
    return 0;
}

bool clean_environment() {
#if defined(__linux__)
    unlink(DB_PATH);

    char path_buf[256];

    snprintf(path_buf, sizeof(path_buf), "%s-wal", DB_PATH);
    unlink(path_buf);

    snprintf(path_buf, sizeof(path_buf), "%s-shm", DB_PATH);
    unlink(path_buf);

    return true;
#else
    printf(COLOR_YELLOW "the requrested operation is currently not supported for non-linux "
                        "Operating Systems\n" COLOR_RESET);
    return false;
#endif
}
bool init_test() {
    if (!clean_environment()) {
        printf(COLOR_YELLOW "an error occured when cleaning the environment, you may encounter an "
                            "unaccurate reports\n" COLOR_RESET);
    }
    if (sqlite3_open(DB_PATH, &db) != SQLITE_OK) {
        return false;
    }

    return db;
}

bool test_repo_init() {
    repo_return_code rc = repo_init(db);

    switch (rc) {
        case DATA_BASE_ERR:
            return false;
        case OK:
            return true;
        default:;
    }
}

static IntVaultEntry *create_entry() {
#define ENTRIES_LEN 10
    IntVaultEntry *out_entry = malloc(sizeof(IntVaultEntry));

    out_entry->uuid = malloc(sizeof(char) * (UUID_STR_LEN + 1));
    generate_uuid(out_entry->uuid);

    out_entry->service_name = malloc(sizeof(char) * (out_entry->service_len = ENTRIES_LEN));
    generate_password(ENTRIES_LEN, (char *)out_entry->service_name, CHARSET_ALPHA);

    out_entry->username = malloc(sizeof(char) * (out_entry->username_len = ENTRIES_LEN));
    generate_password(ENTRIES_LEN, (char *)out_entry->username, CHARSET_ALPHA);

    out_entry->password = malloc(sizeof(char) * (out_entry->password_len = ENTRIES_LEN));
    generate_password(ENTRIES_LEN, (char *)out_entry->password, CHARSET_ALPHA);

    out_entry->notes = malloc(sizeof(char) * (out_entry->notes_len = ENTRIES_LEN));
    generate_password(ENTRIES_LEN, (char *)out_entry->notes, CHARSET_ALPHA);

    out_entry->created_at = time(NULL);
    out_entry->updated_at = out_entry->created_at;

    return out_entry;
}
static bool compare_blob(const uint8_t *a, uint32_t a_len, const uint8_t *b, uint32_t b_len) {
    if (a_len != b_len) {
        return false;
    }
    if (a_len == 0) {
        return true;
    }
    if (!a || !b) {
        return false;
    }
    return memcmp(a, b, a_len) == 0;
}

static bool entries_are_equal(const IntVaultEntry *e1, const IntVaultEntry *e2) {
    if (e1 == e2)
        return true;
    if (!e1 || !e2)
        return false;
    if (e1->created_at != e2->created_at)
        return false;
    if (e1->updated_at != e2->updated_at)
        return false;
    if (e1->uuid == NULL || e2->uuid == NULL) {
        if (e1->uuid != e2->uuid)
            return false;
    } else {
        if (strcmp(e1->uuid, e2->uuid) != 0)
            return false;
    }

    if (!compare_blob(e1->service_name, e1->service_len, e2->service_name, e2->service_len))
        return false;
    if (!compare_blob(e1->username, e1->username_len, e2->username, e2->username_len))
        return false;
    if (!compare_blob(e1->password, e1->password_len, e2->password, e2->password_len))
        return false;
    if (!compare_blob(e1->notes, e1->notes_len, e2->notes, e2->notes_len))
        return false;

    return true;
}
bool test_add_entry() {
    entry1 = create_entry();
    entry2 = create_entry();

    if (add_entry(entry1, db) != OK) {
        printf("error adding entry 1\n");
        return false;
    }

    if (add_entry(entry2, db) != OK) {
        printf("error adding entry 2\n");
        return false;
    }

    return true;
}
static void print_entry(IntVaultEntry *entry) {
    if (!entry) {
        printf("Entry is NULL.\n");
        return;
    }

    printf("+----------------+------------------------------------------+\n");
    printf("| %-14s | %-40s |\n", "Field", "Value");
    printf("+----------------+------------------------------------------+\n");
    printf("| %-14s | %-40s |\n", "UUID", entry->uuid ? entry->uuid : "(null)");
    printf("| %-14s | %-40.*s |\n", "Service", (int)entry->service_len,
           (char *)entry->service_name);
    printf("| %-14s | %-40.*s |\n", "Username", (int)entry->username_len, (char *)entry->username);
    printf("| %-14s | %-40.*s |\n", "Password", (int)entry->password_len, (char *)entry->password);
    printf("| %-14s | %-40.*s |\n", "Notes", (int)entry->notes_len, (char *)entry->notes);
    printf("| %-14s | %-40" PRIu64 " |\n", "Created At", entry->created_at);
    printf("| %-14s | %-40" PRIu64 " |\n", "Updated At", entry->updated_at);
    printf("+----------------+------------------------------------------+\n");
}
bool test_read_entry() {
    IntVaultEntry *readEntry = malloc(sizeof(IntVaultEntry));

    repo_return_code rc = read_entry(entry1->uuid, readEntry, db);
    switch (rc) {
        case OK:
            if (!entries_are_equal(readEntry, entry1)) {
                printf("the readed entry and the expected one are not equal\n");
                return false;
            } else {
                printf("the readed entry : \n");
                print_entry(readEntry);
                return true;
            }
        case DATA_BASE_ERR:
            op_status = DATA_BASE_ERR;
            return false;
        case MEMORY_ERR:
            op_status = MEMORY_ERR;
            return false;
        case REPO_UNEXPECTED_ERR:
            op_status = REPO_UNEXPECTED_ERR;
            return false;
        default:;
    }

    return true;
}

static void print_linked_list_entries(const DLinkedList *list) {
    if (!list) {
        printf("[DEBUG] List container is NULL.\n");
        return;
    }

    if (!list->head) {
        printf("[DEBUG] List is empty (Head is NULL).\n");
        printf("Size reported: %" PRIu64 "\n", list->size);
        return;
    }

    DLinkedListNode *current = list->head;
    int index = 0;

    printf("+----------------+------------------------------------------------------+\n");
    printf("| %-14s | %-52s |\n", "FIELD", "VALUE (Integrity Check)");
    printf("+----------------+------------------------------------------------------+\n");

    while (current != NULL) {
        IntVaultEntry *entry = (IntVaultEntry *)current->data;

        printf("| %-14s | Idx: %-3d | Curr: %p                      |\n", "== NODE ==", index,
               (void *)current);
        printf("| %-14s | Prev: %p | Next: %p                   |\n", "", (void *)current->prev,
               (void *)current->next);
        printf("|----------------|------------------------------------------------------|\n");
        if (entry) {
            printf("| %-14s | %-52s |\n", "UUID", entry->uuid ? entry->uuid : "(null)");
            printf("| %-14s | (len:%-3u) %-42.*s |\n", "Service", entry->service_len,
                   (int)entry->service_len, (char *)entry->service_name);
            printf("| %-14s | (len:%-3u) %-42.*s |\n", "Username", entry->username_len,
                   (int)entry->username_len, (char *)entry->username);
            printf("| %-14s | (len:%-3u) %-42.*s |\n", "Password", entry->password_len,
                   (int)entry->password_len, (char *)entry->password);
            printf("| %-14s | (len:%-3u) %-42.*s |\n", "Notes", entry->notes_len,
                   (int)entry->notes_len, (char *)entry->notes);
            printf("| %-14s | %-52" PRIu64 " |\n", "Created_at", entry->created_at);
            printf("| %-14s | %-52" PRIu64 " |\n", "Updated_at", entry->updated_at);
        } else {
            printf("| %-14s | %-52s |\n", "DATA", "NULL POINTER (Corrupted Node?)");
        }
        printf("+----------------+------------------------------------------------------+\n");

        current = current->next;
        index++;
    }
}

bool test_read_all_entries() {
    DLinkedList *wrapper = dlinked_list_create();

    repo_return_code rc = read_all_entries(wrapper, db);

    if (wrapper->size != 2) {
        printf("the size is not 2\n");
        return false;
    }

    switch (rc) {
        case OK:
            print_linked_list_entries(wrapper);
            return true;

        case DATA_BASE_ERR:
            op_status = DATA_BASE_ERR;
            return false;

        case MEMORY_ERR:
            op_status = MEMORY_ERR;
            return false;

        case DATA_STRUCTURE_ERR:
            op_status = DATA_STRUCTURE_ERR;
            return false;

        case REPO_UNEXPECTED_ERR:
            op_status = REPO_UNEXPECTED_ERR;
            return false;

        default:;
    }

    return true;
}
bool test_update_entry() {
    IntVaultEntry *updated_entry = malloc(sizeof(IntVaultEntry));

    updated_entry->uuid = malloc(sizeof(char) * (UUID_STR_LEN + 1));
    strcpy(updated_entry->uuid, entry1->uuid);

    updated_entry->service_name = malloc(sizeof(char) * (updated_entry->service_len = 10));
    generate_password(10, (char *)updated_entry->service_name, CHARSET_ALPHA);

    updated_entry->username = malloc(sizeof(char) * (updated_entry->username_len = 10));
    generate_password(10, (char *)updated_entry->username, CHARSET_ALPHA);

    updated_entry->password = malloc(sizeof(char) * (updated_entry->password_len = 10));
    generate_password(10, (char *)updated_entry->password, CHARSET_ALPHA);

    updated_entry->notes = malloc(sizeof(char) * (updated_entry->notes_len = 10));
    generate_password(10, (char *)updated_entry->notes, CHARSET_ALPHA);

    updated_entry->created_at = entry1->created_at;
    updated_entry->updated_at = time(NULL);

    repo_return_code rc = update_entry(entry1->uuid, updated_entry, db);

    switch (rc) {
        case OK: {
            IntVaultEntry *verify_entry = malloc(sizeof(IntVaultEntry));
            repo_return_code verify_rc = read_entry(entry1->uuid, verify_entry, db);
            if (verify_rc == OK && entries_are_equal(verify_entry, updated_entry)) {
                printf("Entry updated and verified successfully:\n");
                print_entry(verify_entry);
                free(verify_entry);
                free(updated_entry->uuid);
                free(updated_entry->service_name);
                free(updated_entry->username);
                free(updated_entry->password);
                free(updated_entry->notes);
                free(updated_entry);
                return true;
            }
            printf("Updated entry does not match verified entry\n");
            free(verify_entry);
            return false;
        }
        case DATA_BASE_ERR:
            op_status = DATA_BASE_ERR;
            return false;
        case MEMORY_ERR:
            op_status = MEMORY_ERR;
            return false;
        case NOT_FOUND_ERR:
            op_status = NOT_FOUND_ERR;
            return false;
        case REPO_UNEXPECTED_ERR:
            op_status = REPO_UNEXPECTED_ERR;
            return false;
        default:
            return false;
    }
}
bool test_delete_entry() {
    repo_return_code rc = delete_entry(entry2->uuid, db);

    switch (rc) {
        case OK: {
            IntVaultEntry *verify_entry = malloc(sizeof(IntVaultEntry));
            repo_return_code verify_rc = read_entry(entry2->uuid, verify_entry, db);
            if (verify_rc == NOT_FOUND_ERR) {
                printf("Entry deleted successfully (verified not found):\n");
                printf("Deleted UUID: %s\n", entry2->uuid);
                free(verify_entry);
                return true;
            }
            printf("Entry was not actually deleted\n");
            free(verify_entry);
            return false;
        }
        case DATA_BASE_ERR:
            op_status = DATA_BASE_ERR;
            return false;
        case NOT_FOUND_ERR:
            op_status = NOT_FOUND_ERR;
            return false;
        case REPO_UNEXPECTED_ERR:
            op_status = REPO_UNEXPECTED_ERR;
            return false;
        default:
            return false;
    }
}
bool test_delete_all_entries() {
    repo_return_code rc = delete_all_entries(db);

    switch (rc) {
        case OK: {
            DLinkedList *verify_list = dlinked_list_create();
            repo_return_code verify_rc = read_all_entries(verify_list, db);
            if (verify_rc == OK && verify_list->size == 0) {
                printf("All entries deleted successfully (verified empty):\n");
                printf("List size after delete_all: %" PRIu64 "\n", verify_list->size);
                dlinked_list_destroy(verify_list, NULL);
                return true;
            }
            printf("Entries still exist after delete_all\n");
            printf("List size: %" PRIu64 "\n", verify_list->size);
            dlinked_list_destroy(verify_list, NULL);
            return false;
        }
        case DATA_BASE_ERR:
            op_status = DATA_BASE_ERR;
            return false;
        case REPO_UNEXPECTED_ERR:
            op_status = REPO_UNEXPECTED_ERR;
            return false;
        default:
            return false;
    }
}

bool close_test() {
    if (!clean_environment()) {
        printf(COLOR_YELLOW "an error occured when cleaning the environment\n" COLOR_RESET);
        return false;
    }
    if (sqlite3_close(db) != SQLITE_OK) {
        return false;
    }
    return true;
}

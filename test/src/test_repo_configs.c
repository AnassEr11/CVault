#include <CVault/repository/repository.h>
#include <CVault/utils/security_utils.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
Config *config1;
Config *config2;
repo_return_code op_status;

bool clean_environment();
bool init_test();
bool test_repo_init();
bool test_add_config();
bool test_read_config();
bool test_update_config();
bool test_delete_config();
bool test_delete_all_configs();

bool close_test();

int main() {
    printf("\n%sTEST CONFIGS REPOSITORY OPERATIONS%s\n\n", COLOR_BLUE, COLOR_RESET);

    printf("%s[TEST 1/7]%s Initializing database...\n", COLOR_BLUE, COLOR_RESET);
    if (!init_test()) {
        printf("%s[FAILED]%s Database initialization failed\n\n", COLOR_RED, COLOR_RESET);
        return 1;
    }
    printf("%s[PASSED]%s Database initialized successfully\n\n", COLOR_GREEN, COLOR_RESET);

    printf("%s[TEST 2/7]%s Initializing repository...\n", COLOR_BLUE, COLOR_RESET);
    if (!test_repo_init()) {
        printf("%s[FAILED]%s Repository initialization failed\n\n", COLOR_RED, COLOR_RESET);
        sqlite3_close(db);
        return 1;
    }
    printf("%s[PASSED]%s Repository initialized successfully\n\n", COLOR_GREEN, COLOR_RESET);

    printf("%s[TEST 3/7]%s Adding configs...\n", COLOR_BLUE, COLOR_RESET);
    if (!test_add_config()) {
        printf("%s[FAILED]%s Failed to add configs\n\n", COLOR_RED, COLOR_RESET);
        sqlite3_close(db);
        return 1;
    }
    printf("%s[PASSED]%s Configs added successfully\n\n", COLOR_GREEN, COLOR_RESET);

    printf("%s[TEST 4/7]%s Reading config...\n", COLOR_BLUE, COLOR_RESET);
    if (!test_read_config()) {
        printf("%s[FAILED]%s Failed to read config\n\n", COLOR_RED, COLOR_RESET);
        sqlite3_close(db);
        return 1;
    }
    printf("%s[PASSED]%s Config read successfully\n\n", COLOR_GREEN, COLOR_RESET);

    printf("%s[TEST 5/7]%s Updating config...\n", COLOR_BLUE, COLOR_RESET);
    if (!test_update_config()) {
        printf("%s[FAILED]%s Failed to update config\n\n", COLOR_RED, COLOR_RESET);
        sqlite3_close(db);
        return 1;
    }
    printf("%s[PASSED]%s Config updated successfully\n\n", COLOR_GREEN, COLOR_RESET);

    printf("%s[TEST 6/7]%s Deleting config...\n", COLOR_BLUE, COLOR_RESET);
    if (!test_delete_config()) {
        printf("%s[FAILED]%s Failed to delete config\n\n", COLOR_RED, COLOR_RESET);
        sqlite3_close(db);
        return 1;
    }
    printf("%s[PASSED]%s Config deleted successfully\n\n", COLOR_GREEN, COLOR_RESET);

    printf("%s[TEST 7/7]%s Deleting all configs...\n", COLOR_BLUE, COLOR_RESET);
    if (!test_delete_all_configs()) {
        printf("%s[FAILED]%s Failed to delete all configs\n\n", COLOR_RED, COLOR_RESET);
        sqlite3_close(db);
        return 1;
    }
    printf("%s[PASSED]%s All configs deleted successfully\n\n", COLOR_GREEN, COLOR_RESET);

    printf("%s[CLEANUP]%s Closing database...\n", COLOR_YELLOW, COLOR_RESET);
    if (!close_test()) {
        printf("%s[WARNING]%s Database close failed\n\n", COLOR_YELLOW, COLOR_RESET);
    } else {
        printf("%s[CLEANUP DONE]%s Database closed successfully\n\n", COLOR_GREEN, COLOR_RESET);
    }

    printf("%sCONFIGS REPO OPERATIONS TEST COMPLETED%s\n", COLOR_BLUE, COLOR_RESET);
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

static Config *create_config() {
#define CONFIG_LEN 10
    Config *out_config = malloc(sizeof(Config));

    out_config->config_key = malloc(sizeof(char) * (CONFIG_LEN + 1));
    generate_password(CONFIG_LEN, out_config->config_key, CHARSET_ALPHA);

    out_config->config_value_len = CONFIG_LEN;
    out_config->config_value = malloc(CONFIG_LEN);
    generate_password(CONFIG_LEN, (char *)out_config->config_value, CHARSET_ALPHA);

    return out_config;
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

static bool configs_are_equal(const Config *c1, const Config *c2) {
    if (c1 == c2)
        return true;
    if (!c1 || !c2)
        return false;

    if (c1->config_key == NULL || c2->config_key == NULL) {
        if (c1->config_key != c2->config_key)
            return false;
    } else {
        if (strcmp(c1->config_key, c2->config_key) != 0)
            return false;
    }

    if (!compare_blob(c1->config_value, c1->config_value_len, c2->config_value,
                      c2->config_value_len))
        return false;

    return true;
}

bool test_add_config() {
    config1 = create_config();
    config2 = create_config();

    if (add_config(config1, db) != OK) {
        printf("error adding config 1\n");
        return false;
    }

    if (add_config(config2, db) != OK) {
        printf("error adding config 2\n");
        return false;
    }

    return true;
}

static void print_config(Config *config) {
    if (!config) {
        printf("Config is NULL.\n");
        return;
    }

    printf("+----------------+------------------------------------------+\n");
    printf("| %-14s | %-40s |\n", "Field", "Value");
    printf("+----------------+------------------------------------------+\n");
    printf("| %-14s | %-40s |\n", "Key", config->config_key ? config->config_key : "(null)");
    printf("| %-14s | (len:%-3u) %-34.*s |\n", "Value", config->config_value_len,
           (int)config->config_value_len, (char *)config->config_value);
    printf("+----------------+------------------------------------------+\n");
}

bool test_read_config() {
    Config *readConfig = malloc(sizeof(Config));

    repo_return_code rc = read_config(config1->config_key, readConfig, db);
    switch (rc) {
        case OK:
            if (!configs_are_equal(readConfig, config1)) {
                printf("the readed config and the expected one are not equal\n");
                return false;
            } else {
                printf("the readed config : \n");
                print_config(readConfig);
                free(readConfig->config_key);
                free(readConfig->config_value);
                free(readConfig);
                return true;
            }
        case DATA_BASE_ERR:
            op_status = DATA_BASE_ERR;
            return false;
        case NOT_FOUND_ERR:
            op_status = NOT_FOUND_ERR;
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

bool test_update_config() {
    Config *updated_config = malloc(sizeof(Config));

    updated_config->config_key = malloc(sizeof(char) * (CONFIG_LEN + 1));
    strcpy(updated_config->config_key, config1->config_key);

    updated_config->config_value_len = CONFIG_LEN;
    updated_config->config_value = malloc(CONFIG_LEN);
    generate_password(CONFIG_LEN, (char *)updated_config->config_value, CHARSET_ALPHA);

    repo_return_code rc = update_config(config1->config_key, updated_config, db);

    switch (rc) {
        case OK: {
            Config *verify_config = malloc(sizeof(Config));
            repo_return_code verify_rc = read_config(config1->config_key, verify_config, db);
            if (verify_rc == OK && configs_are_equal(verify_config, updated_config)) {
                printf("Config updated and verified successfully:\n");
                print_config(verify_config);
                free(verify_config->config_key);
                free(verify_config->config_value);
                free(verify_config);
                free(updated_config->config_key);
                free(updated_config->config_value);
                free(updated_config);
                return true;
            }
            printf("Updated config does not match verified config\n");
            free(verify_config->config_key);
            free(verify_config->config_value);
            free(verify_config);
            return false;
        }
        case DATA_BASE_ERR:
            op_status = DATA_BASE_ERR;
            return false;
        case NOT_FOUND_ERR:
            op_status = NOT_FOUND_ERR;
            return false;
        case MEMORY_ERR:
            op_status = MEMORY_ERR;
            return false;
        case REPO_UNEXPECTED_ERR:
            op_status = REPO_UNEXPECTED_ERR;
            return false;
        default:
            return false;
    }
}

bool test_delete_config() {
    repo_return_code rc = delete_configs(config2->config_key, db);

    switch (rc) {
        case OK: {
            Config *verify_config = malloc(sizeof(Config));
            repo_return_code verify_rc = read_config(config2->config_key, verify_config, db);
            if (verify_rc == NOT_FOUND_ERR) {
                printf("Config deleted successfully (verified not found):\n");
                printf("Deleted Key: %s\n", config2->config_key);
                free(verify_config);
                return true;
            }
            printf("Config was not actually deleted\n");
            free(verify_config);
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

bool test_delete_all_configs() {
    repo_return_code rc = delete_all_configs(db);

    switch (rc) {
        case OK:
            Config *verify_config = malloc(sizeof(Config));
            repo_return_code verify_rc = read_config(config1->config_key, verify_config, db);
            if (verify_rc == NOT_FOUND_ERR) {
                printf("All configs deleted successfully (verified empty):\n");
                free(verify_config);
                return true;
            }
            printf("Configs still exist after delete_all\n");
            free(verify_config);
            return false;

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
    if (config1) {
        free(config1->config_key);
        free(config1->config_value);
        free(config1);
    }
    if (config2) {
        free(config2->config_key);
        free(config2->config_value);
        free(config2);
    }

    if (db) {
        if (sqlite3_close(db) != SQLITE_OK) {
            return false;
        }
    }

    if (!clean_environment()) {
        printf(COLOR_YELLOW "an error occured when cleaning the environment\n" COLOR_RESET);
        return false;
    }
    return true;
}

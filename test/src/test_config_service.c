#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <CVault/service/db_init_service.h>
#include <CVault/service/db_config_service.h>
#include <CVault/models/config.h>

#define COLOR_RESET  "\033[0m"
#define COLOR_GREEN  "\033[0;32m"
#define COLOR_RED    "\033[0;31m"
#define COLOR_BLUE   "\033[34m"
#define COLOR_YELLOW "\033[1;33m"
#define COLOR_CYAN   "\033[0;36m"

static void print_result(const char* test_name, bool success);
static bool test_add_config();
static bool test_read_config();
static bool test_update_config();
static bool test_delete_config();
static bool test_delete_all_configs();

int main() {
    printf(COLOR_BLUE "\n=== CONFIG SERVICE TEST ===\n\n" COLOR_RESET);

    printf(COLOR_YELLOW "--> Initializing schema...\n" COLOR_RESET);
    if (!init_schema()) {
        printf(COLOR_RED ">> Failed to initialize schema\n" COLOR_RESET);
        return 1;
    }
    printf(COLOR_GREEN ">> Schema initialized successfully\n\n" COLOR_RESET);

    printf(COLOR_YELLOW "--> Opening config service...\n" COLOR_RESET);
    if (!open_config_service()) {
        printf(COLOR_RED ">> Failed to open config service\n" COLOR_RESET);
        return 1;
    }
    printf(COLOR_GREEN ">> Config service opened successfully\n\n" COLOR_RESET);

    printf(COLOR_BLUE "[TEST 1/5] Adding config...\n" COLOR_RESET);
    if (!test_add_config()) {
        printf(COLOR_RED "[FAILED] Failed to add config\n\n" COLOR_RESET);
        close_config_service();
        return 1;
    }
    printf(COLOR_GREEN "[PASSED] Config added successfully\n\n" COLOR_RESET);

    printf(COLOR_BLUE "[TEST 2/5] Reading config...\n" COLOR_RESET);
    if (!test_read_config()) {
        printf(COLOR_RED "[FAILED] Failed to read config\n\n" COLOR_RESET);
        close_config_service();
        return 1;
    }
    printf(COLOR_GREEN "[PASSED] Config read successfully\n\n" COLOR_RESET);

    printf(COLOR_BLUE "[TEST 3/5] Updating config...\n" COLOR_RESET);
    if (!test_update_config()) {
        printf(COLOR_RED "[FAILED] Failed to update config\n\n" COLOR_RESET);
        close_config_service();
        return 1;
    }
    printf(COLOR_GREEN "[PASSED] Config updated successfully\n\n" COLOR_RESET);

    printf(COLOR_BLUE "[TEST 4/5] Deleting single config...\n" COLOR_RESET);
    if (!test_delete_config()) {
        printf(COLOR_RED "[FAILED] Failed to delete config\n\n" COLOR_RESET);
        close_config_service();
        return 1;
    }
    printf(COLOR_GREEN "[PASSED] Config deleted successfully\n\n" COLOR_RESET);

    printf(COLOR_BLUE "[TEST 5/5] Deleting all configs...\n" COLOR_RESET);
    if (!test_delete_all_configs()) {
        printf(COLOR_RED "[FAILED] Failed to delete all configs\n\n" COLOR_RESET);
        close_config_service();
        return 1;
    }
    printf(COLOR_GREEN "[PASSED] All configs deleted successfully\n\n" COLOR_RESET);

    printf(COLOR_YELLOW "--> Closing config service...\n" COLOR_RESET);
    if (!close_config_service()) {
        printf(COLOR_YELLOW ">> Warning: Config service close failed\n" COLOR_RESET);
    } else {
        printf(COLOR_GREEN ">> Config service closed successfully\n\n" COLOR_RESET);
    }

    printf(COLOR_BLUE "=== CONFIG SERVICE TEST COMPLETED ===\n\n" COLOR_RESET);
    return 0;
}

static bool test_add_config() {
    Config cfg;
    cfg.config_key = "app_theme";
    cfg.config_value = (uint8_t *)"dark_mode";
    cfg.config_value_len = strlen("dark_mode");

    if (!service_add_config(&cfg)) {
        printf(COLOR_RED ">> Failed to add config\n" COLOR_RESET);
        return false;
    }

    Config cfg2;
    cfg2.config_key = "language";
    cfg2.config_value = (uint8_t *)"en_US";
    cfg2.config_value_len = strlen("en_US");

    if (!service_add_config(&cfg2)) {
        printf(COLOR_RED ">> Failed to add second config\n" COLOR_RESET);
        return false;
    }

    printf(COLOR_CYAN ">> Added 2 configs: 'app_theme' and 'language'\n" COLOR_RESET);
    return true;
}

static bool test_read_config() {
    Config read_cfg;
    read_cfg.config_value = malloc(256);

    if (!service_read_config("app_theme", &read_cfg)) {
        printf(COLOR_RED ">> Failed to read config 'app_theme'\n" COLOR_RESET);
        free(read_cfg.config_value);
        return false;
    }

    if (read_cfg.config_value_len != strlen("dark_mode") ||
        memcmp(read_cfg.config_value, "dark_mode", read_cfg.config_value_len) != 0) {
        printf(COLOR_RED ">> Config value mismatch\n" COLOR_RESET);
        free(read_cfg.config_value);
        return false;
    }

    printf(COLOR_CYAN ">> Read config 'app_theme': %.*s\n" COLOR_RESET,
           (int)read_cfg.config_value_len, (char *)read_cfg.config_value);
    free(read_cfg.config_value);
    return true;
}

static bool test_update_config() {
    uint8_t new_value[] = "light_mode";
    size_t new_size = strlen((char *)new_value);

    if (!service_update_config("app_theme", new_value, new_size)) {
        printf(COLOR_RED ">> Failed to update config 'app_theme'\n" COLOR_RESET);
        return false;
    }

    Config verify_cfg;
    verify_cfg.config_value = malloc(256);

    if (!service_read_config("app_theme", &verify_cfg)) {
        printf(COLOR_RED ">> Failed to verify updated config\n" COLOR_RESET);
        free(verify_cfg.config_value);
        return false;
    }

    if (memcmp(verify_cfg.config_value, new_value, new_size) != 0) {
        printf(COLOR_RED ">> Updated value does not match\n" COLOR_RESET);
        free(verify_cfg.config_value);
        return false;
    }

    printf(COLOR_CYAN ">> Updated 'app_theme' to: %.*s\n" COLOR_RESET,
           (int)verify_cfg.config_value_len, (char *)verify_cfg.config_value);
    free(verify_cfg.config_value);
    return true;
}

static bool test_delete_config() {
    if (!service_delete_config("language")) {
        printf(COLOR_RED ">> Failed to delete config 'language'\n" COLOR_RESET);
        return false;
    }

    Config verify_cfg;
    verify_cfg.config_value = malloc(256);

    if (service_read_config("language", &verify_cfg)) {
        printf(COLOR_RED ">> Config 'language' still exists after deletion\n" COLOR_RESET);
        free(verify_cfg.config_value);
        return false;
    }

    printf(COLOR_CYAN ">> Successfully deleted config 'language'\n" COLOR_RESET);
    free(verify_cfg.config_value);
    return true;
}

static bool test_delete_all_configs() {
    if (!service_delete_all_configs()) {
        printf(COLOR_RED ">> Failed to delete all configs\n" COLOR_RESET);
        return false;
    }

    Config verify_cfg;
    verify_cfg.config_value = malloc(256);

    if (service_read_config("app_theme", &verify_cfg)) {
        printf(COLOR_RED ">> Configs still exist after delete_all\n" COLOR_RESET);
        free(verify_cfg.config_value);
        return false;
    }

    printf(COLOR_CYAN ">> Successfully deleted all configs\n" COLOR_RESET);
    free(verify_cfg.config_value);
    return true;
}

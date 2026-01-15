#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>
#include <CVault/utils/security_utils.h>

#define COLOR_RESET  "\033[0m"
#define COLOR_GREEN  "\033[0;32m"
#define COLOR_RED    "\033[0;31m"
#define COLOR_BLUE   "\033[34m"
#define COLOR_YELLOW "\033[1;33m"
#define COLOR_CYAN   "\033[0;36m"

const char* util_code_to_str(util_result_code code);
static void print_result(const char* test_name, util_result_code code);

int main() {
    printf(COLOR_BLUE "\nSECURITY UTILS TEST\n" COLOR_RESET);

    printf(COLOR_YELLOW "\n--> Testing UUID Generation\n" COLOR_RESET);
    char uuid_buf[UUID_STR_LEN + 1];
    util_result_code res = generate_uuid(uuid_buf);
    print_result("Generate UUID", res);
    if (res == SUCCESS) {
        printf(COLOR_CYAN "Generated UUID: %s\n" COLOR_RESET, uuid_buf);
        if (strlen(uuid_buf) == UUID_STR_LEN && uuid_buf[14] == '4') {
            printf(COLOR_GREEN ">> UUID Format and Version 4 check passed\n" COLOR_RESET);
        } else {
            printf(COLOR_RED ">> UUID Format check failed\n" COLOR_RESET);
        }
    }

    printf(COLOR_YELLOW "\n--> Testing Password Generation\n" COLOR_RESET);
    char pass_buf[MAX_LEN + 1];
    uint64_t lengths[] = {8, 16, 32};
    charset_flag charsets[] = {CHARSET_FULL, CHARSET_ALPHANUM, CHARSET_DIGITS};
    const char* charset_names[] = {"FULL", "ALPHANUM", "DIGITS"};

    for (int i = 0; i < 3; i++) {
        res = generate_password(lengths[i], pass_buf, charsets[i]);
        char test_label[50];
        sprintf(test_label, "Generate %lu char pass (%s)", lengths[i], charset_names[i]);
        print_result(test_label, res);
        if (res == SUCCESS) {
            printf(COLOR_CYAN "Generated Pass: %s\n" COLOR_RESET, pass_buf);
        }
    }

    printf(COLOR_YELLOW "\n--> Testing Secure Memset\n" COLOR_RESET);
    char sensitive_data[] = "SuperSecret123";
    size_t data_len = strlen(sensitive_data);
    
    printf("Data before: %s\n", sensitive_data);
    res = secure_memset(sensitive_data, data_len);
    print_result("Secure Memset", res);
    
    bool is_cleared = true;
    for (size_t i = 0; i < data_len; i++) {
        if (sensitive_data[i] != 0) is_cleared = false;
    }
    
    if (is_cleared) {
        printf(COLOR_GREEN ">> Memory successfully zeroed\n" COLOR_RESET);
    } else {
        printf(COLOR_RED ">> Memory NOT cleared properly\n" COLOR_RESET);
    }

    printf(COLOR_YELLOW "\n--> Testing Clipboard (wl-copy)\n" COLOR_RESET);
    printf(COLOR_CYAN "Note: This will copy 'CVault-Test-Token' to your clipboard.\n" COLOR_RESET);
    printf(COLOR_CYAN "It will be cleared automatically in %d seconds.\n" COLOR_RESET, TIMEOUT);
    
    char* test_clip = "CVault-Test-Token";
    res = copy_string_to_clipboard(test_clip);
    
    if (res == NOT_FOUND) {
        printf(COLOR_RED "[SKIP] 'wl-copy' not found. Are you on Wayland?\n" COLOR_RESET);
    } else {
        print_result("Copy to Clipboard", res);
        if (res == SUCCESS) {
            printf(COLOR_GREEN ">> Check your clipboard now!\n" COLOR_RESET);
            sleep(1);
        }
    }

    printf(COLOR_BLUE "\nSECURITY UTILS TEST COMPLETED\n\n" COLOR_RESET);
    return 0;
}

const char* util_code_to_str(util_result_code code) {
    switch (code) {
        case SUCCESS:        return "SUCCESS";
        case NOT_SUPPORTED:  return "NOT_SUPPORTED";
        case NULL_POINTER:   return "NULL_POINTER";
        case INVALID_SIZE:   return "INVALID_SIZE";
        case SYSCALL_ERR:    return "SYSCALL_ERR";
        case UNEXPECTED_ERR: return "UNEXPECTED_ERR";
        case NOT_FOUND:      return "NOT_FOUND";
        default:             return "UNKNOWN_ERROR";
    }
}
static void print_result(const char* test_name, util_result_code code) {
    if (code == SUCCESS) {
        printf(COLOR_GREEN "[PASS] " COLOR_RESET "%s\n", test_name);
    } else {
		printf(COLOR_RED "[FAIL] " COLOR_RESET "%-30s " COLOR_RED "(%s)" COLOR_RESET "\n",
		 test_name, util_code_to_str(code));
	}
}

#include <CVault/utils/security_utils.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#if defined(__linux__)
#include <signal.h>
#include <sys/random.h>
#include <sys/wait.h>
#include <unistd.h>
#endif

util_result_code secure_memset(void *ptr, uint64_t width) {
    if (!ptr) {
        return NULL_POINTER;
    }

    if (width <= 0 || width > MAX_LEN) {
        return INVALID_SIZE;
    }

    volatile uint8_t *p = (volatile uint8_t *)ptr;

    while (width--) {
        *p++ = 0;
    }

    return SUCCESS;
}

util_result_code generate_uuid(char *out_buffer) {
#if defined(__linux__)

    if (!out_buffer) {
        return NULL_POINTER;
    }

    uint8_t random_bytes[16];

    if (getrandom(random_bytes, 16, 0) != 16) {
        return SYSCALL_ERR;
    }

    random_bytes[6] = (random_bytes[6] & 0x0F) | 0x40;
    random_bytes[8] = (random_bytes[8] & 0x3F) | 0x80;

    snprintf(out_buffer, UUID_STR_LEN + 1,
             "%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x",
             random_bytes[0], random_bytes[1], random_bytes[2], random_bytes[3], random_bytes[4],
             random_bytes[5], random_bytes[6], random_bytes[7], random_bytes[8], random_bytes[9],
             random_bytes[10], random_bytes[11], random_bytes[12], random_bytes[13],
             random_bytes[14], random_bytes[15]);

    return SUCCESS;

#else
    // TODO: add portability to other platforms
    return NOT_SUPPORTED;
#endif
}

util_result_code generate_password(uint64_t length, char *out_buffer, charset_flag flag) {
    if (!out_buffer) {
        return NULL_POINTER;
    }

    if (length <= 0 || length > MAX_LEN) {
        return INVALID_SIZE;
    }

    char *charset;
    int charset_size;

    switch (flag) {
        default:
        case CHARSET_FULL:
            charset = "abcdefghijklmnopqrstuvwxyz"
                      "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                      "0123456789"
                      "!@_#)$%=^+&*(-";
            charset_size = 76;
            break;
        case CHARSET_ALPHANUM:
            charset = "abcdefghijklmnopqrstuvwxyz"
                      "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                      "0123456789";
            charset_size = 62;
            break;
        case CHARSET_ALPHA:
            charset = "abcdefghijklmnopqrstuvwxyz"
                      "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
            charset_size = 52;
            break;
        case CHARSET_UPPER:
            charset = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
            charset_size = 26;
            break;
        case CHARSET_LOWER:
            charset = "abcdefghijklmnopqrstuvwxyz";
            charset_size = 26;
            break;
        case CHARSET_DIGITS_SYMBOLS:
            charset = "0123456789"
                      "!@#$%^&*()-_=+";
            charset_size = 24;
            break;
        case CHARSET_SYMBOLS:
            charset = "!@#$%^&*()-_=+";
            charset_size = 14;
            break;
        case CHARSET_DIGITS:
            charset = "0123456789";
            charset_size = 10;
            break;
    }

#if defined(__linux__)
    const uint16_t max_rand = 256;
    const uint16_t bias_threshold = max_rand - (max_rand % charset_size);

    for (uint64_t i = 0; i < length; i++) {
        uint8_t rand_byte;
        do {
            if (getrandom(&rand_byte, 1, 0) != 1) {
                return SYSCALL_ERR;
            }
        } while (rand_byte >= bias_threshold);
        out_buffer[i] = charset[rand_byte % charset_size];
    }
    out_buffer[length] = '\0';
    return SUCCESS;
#else
    // TODO: add portability to other platforms
    return NOT_SUPPORTED;
#endif
}

static pid_t child_process = 0;
static bool clear_clipboard();

util_result_code copy_string_to_clipboard(char *string) {
#if defined(__linux__)
    if (child_process > 0) {
        if (waitpid(child_process, NULL, WNOHANG) > 0) {
            child_process = 0;
        }
    }

    if (child_process > 0) {
        if (kill(child_process, 0) == 0) {
            kill(child_process, SIGKILL);
            waitpid(child_process, NULL, 0);
        }
        child_process = 0;
    }

    FILE *_pipe = popen("wl-copy", "w");
    if (!_pipe) {
        return UNEXPECTED_ERR;
    }
    fprintf(_pipe, "%s", string);

    int status = pclose(_pipe);
    int exit_code = WEXITSTATUS(status);

    if (exit_code == 127) {
        return NOT_FOUND;
    }
    if (exit_code) {
        return UNEXPECTED_ERR;
    }

    pid_t pid = fork();

    if (pid == 0) {
        sleep(TIMEOUT);
        if (!clear_clipboard()) {
            exit(1);
        }
        exit(0);
    } else if (pid > 0) {
        child_process = pid;
        return SUCCESS;
    } else {
        return SYSCALL_ERR;
    }
#else
    // TODO: add portability to other platforms
    return NOT_SUPPORTED;
#endif
}

util_result_code random_raw_bytes(uint64_t size, uint8_t *out_buffer) {
    if (!out_buffer) {
        return NULL_POINTER;
    }

#if defined(__linux__)
    if (getrandom(out_buffer, size, 0) != size) {
        return SYSCALL_ERR;
    }
    return SUCCESS;
#else
    // TODO: add portability to other platforms
    return NOT_SUPPORTED;
#endif
}

bool constant_time_equal(const uint8_t *data1, const uint8_t *data2, size_t length) {
    volatile uint8_t diff = 0;

    for (size_t i = 0; i < length; i++) {
        diff |= data1[i] ^ data2[i];
    }

    return diff == 0;
}

static bool clear_clipboard() {
    FILE *_pipe = popen("wl-copy", "w");
    if (!_pipe) {
        return false;
    }
    fprintf(_pipe, "");
    pclose(_pipe);
    return true;
}

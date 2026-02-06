#include <CVault/crypto/crypto_core.h>
#include <CVault/service/environment_service.h>
#include <CVault/service/titan_key_service.h>
#include <CVault/utils/security_utils.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#if defined(__linux__)

#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#endif

stk_return_code tk_status = 0;

static bool write_to_file(int fd, uint8_t *buffer, size_t length);
static bool read_from_file(int fd, uint8_t *buffer, size_t length);
static bool is_exists_titan_key();

bool init_titan_key() {

    if (is_exists_titan_key() && is_valid_titan_key()) {
        tk_status = TKS_TKFE;
        return false;
    }

    uint8_t titan_key[TITAN_KEY_SIZE_V01];
    if (random_raw_bytes(TITAN_KEY_SIZE_V01, titan_key) != SUCCESS) {
        tk_status = TKS_UTIL_ERR;
        return false;
    }

    if (!initialize_paths()) {
        tk_status = TKS_SERVICE_ERR;
        return false;
    }

#if defined(__linux__)

    int fd = open(titan_key_path, O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR);
    if (fd == -1) {
        tk_status = TKS_SYSCALL_ERR;
        return false;
    }

#else
    // TODO: add portability to other platforms
    tk_status = TKS_UNSOPPORTED_OP;
    return false;
#endif

    int buffer_version_byte = TITAN_KEY_VERSION_01;
    if (!write_to_file(fd, (uint8_t *)&buffer_version_byte, 1)) {
        return false;
    }

    if (!write_to_file(fd, titan_key, TITAN_KEY_SIZE_V01)) {
        return false;
    }

    // TODO: update the following code to read salt from the database
    uint8_t tmp_salt[SALT_LEN] = {0x06};
    uint8_t MAC_buffer[TITAN_KEY_MAC_SIZE_V01];

    if (!hash_key(titan_key, tmp_salt, MAC_buffer)) {
        tk_status = TKS_UTIL_ERR;
        return false;
    }

    if (!write_to_file(fd, MAC_buffer, TITAN_KEY_MAC_SIZE_V01)) {
        return false;
    }

#if defined(__linux__)
    if (close(fd) == -1) {
        tk_status = TKS_SYSCALL_ERR;
        return false;
    }
#else
    // TODO: add portability to other platforms
    tk_status = TKS_UNSOPPORTED_OP;
    return false;
#endif
    secure_memset(titan_key, TITAN_KEY_SIZE_V01);
    secure_memset(MAC_buffer, TITAN_KEY_MAC_SIZE_V01);
    return true;
}

bool load_titan_key(uint8_t *out_titan_key) {
    if (!is_valid_titan_key()) {
        return false;
    }

    if (!out_titan_key) {
        return false;
    }

#if defined(__linux__)
    int fd = open(titan_key_path, O_CREAT | O_RDONLY, S_IRUSR | S_IWUSR);
    if (fd == -1) {
        tk_status = TKS_SYSCALL_ERR;
        return false;
    }

    uint8_t version_byte;

    if (!read_from_file(fd, &version_byte, 1)) {
        return false;
    }

    switch (version_byte) {
        case TITAN_KEY_VERSION_01:
            if (!read_from_file(fd, out_titan_key, TITAN_KEY_SIZE_V01)) {
                return false;
            }

            uint8_t mac[TITAN_KEY_MAC_SIZE_V01] = {0};
            if (!read_from_file(fd, mac, TITAN_KEY_MAC_SIZE_V01)) {
                return false;
            }

            // TODO: modify the following code to read salt from the database
            uint8_t tmp_salt[SALT_LEN] = {0x06};
            uint8_t mac_buffer[TITAN_KEY_MAC_SIZE_V01] = {0};

            if (!hash_key(out_titan_key, tmp_salt, mac_buffer)) {
                tk_status = TKS_UTIL_ERR;
                return false;
            }

            if (constant_time_equal(mac, mac_buffer, TITAN_KEY_MAC_SIZE_V01)) {
                return true;
            } else {
                tk_status = TKS_TAMPERD;
                return false;
            }
        default:
            tk_status = TKS_UNSUPPORTED_VER_ERR;
            return false;
    }

#else
    // TODO: add portability to other platforms
    tk_status = TKS_UNSOPPORTED_OP;
    return false;
#endif /* if defined (__linux__) */
}
bool wipe_titan_key() {
    if (!is_exists_titan_key()) {
        tk_status = TKS_NOTKF;
        return false;
    }
#if defined(__linux__)
    unlink(titan_key_path);
#else
    // TODO: add portability to other platforms
    tk_status = TKS_UNSOPPORTED_OP;
    return false;
#endif /* if defined (__linux__) */
    return true;
}
bool is_valid_titan_key() {

    if (!is_exists_titan_key()) {
        tk_status = TKS_NOTKF;
        return false;
    }

#if defined(__linux__)
    struct stat st;

    if (stat(titan_key_path, &st) == -1) {
        tk_status = TKS_SYSCALL_ERR;
        return false;
    }

    if (st.st_size != TITAN_BLOB_SIZE_V01) {
        tk_status = TKS_TAMPERD;
        return false;
    }

    if ((st.st_mode & S_IFMT) != S_IFREG) {
        tk_status = TKS_TAMPERD;
        return false;
    }

    if ((st.st_mode & (S_IRWXU | S_IRWXG | S_IRWXO)) != (S_IRUSR | S_IWUSR)) {
        tk_status = TKS_TAMPERD;
        return false;
    }

#else
    // TODO: add portability to other platforms
    tk_status = TKS_UNSOPPORTED_OP;
    return false;
#endif /* if defined (__linux__) */

    return true;
}

static bool write_to_file(int fd, uint8_t *buffer, size_t length) {

#if defined(__linux__)

    size_t total_written = 0;
    while (total_written < length) {
        ssize_t bytes_written = write(fd, buffer + total_written, length - total_written);

        if (bytes_written == 0) {
            tk_status = TKS_MEM_ERR;
            return false;
        }

        if (bytes_written == -1) {
            tk_status = TKS_MEM_ERR;
            return false;
        }

        total_written += bytes_written;
    }

#else
    // TODO: add portability to other platforms
    tk_status = TKS_UNSOPPORTED_OP;
    return false;
#endif
    return true;
}

static bool read_from_file(int fd, uint8_t *buffer, size_t length) {

#if defined(__linux__)

    size_t total_read = 0;
    while (total_read < length) {
        ssize_t bytes_read = read(fd, buffer + total_read, length - total_read);

        if (bytes_read == 0) {
            tk_status = TKS_MEM_ERR;
            return false;
        }

        if (bytes_read == -1) {
            tk_status = TKS_MEM_ERR;
            return false;
        }

        total_read += bytes_read;
    }

#else
    // TODO: add portability to other platforms
    tk_status = TKS_UNSOPPORTED_OP;
    return false;
#endif
    return true;
}

static bool is_exists_titan_key() {

    if (!strlen(titan_key_path)) {
        if (!initialize_paths()) {
            return false;
        }
    }

#if defined(__linux__)
    struct stat st;

    if (stat(titan_key_path, &st) == -1) {
        tk_status = TKS_SYSCALL_ERR;
        return false;
    }
#else
    // TODO: add portability to other platforms
    tk_status = TKS_UNSOPPORTED_OP;
    return false;

#endif /* if defined (__linux__) */

    return true;
}

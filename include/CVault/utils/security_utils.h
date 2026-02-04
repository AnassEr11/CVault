#ifndef SECURITY_UTILS_H
#define SECURITY_UTILS_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define UUID_STR_LEN 36
#define TIMEOUT 15
#define MAX_LEN 200

/**
 * @brief: Enumeration for return codes
 */
typedef enum {
    SUCCESS = 0,    /* Operation completed successfully */
    NOT_SUPPORTED,  /* Operation is not supported for the current environment */
    NULL_POINTER,   /* A required pointer argument is NULL*/
    INVALID_SIZE,   /* A length argument exceeds limits (0,MAX_LEN) */
    SYSCALL_ERR,    /*	A system call failed */
    UNEXPECTED_ERR, /* A generic Failure */
    NOT_FOUND       /* A required dependency is not fount */
} util_result_code;

/**
 * @brief: Flags to determine the set used to generate generate
 */
typedef enum {
    CHARSET_FULL,
    CHARSET_ALPHANUM,
    CHARSET_ALPHA,
    CHARSET_UPPER,
    CHARSET_LOWER,
    CHARSET_DIGITS_SYMBOLS,
    CHARSET_SYMBOLS,
    CHARSET_DIGITS
} charset_flag;

/**
 * @brief: Securely wipes memory by bypassing compiler optimizations.
 *
 * @param: ptr The memory block to clear.
 * @param: width The number of bytes to clear.
 *
 * @return: SUCCESS on success,
 * NULL_POINTER on passing null to ptr,
 * INVALID_SIZE on passing an invalid width (too big or negative).
 */
util_result_code secure_memset(void *ptr, uint64_t width);

/**
 * @brief: Generates a standard version 4 UUID string
 *
 * @param: out_buffer Buffer to store the result
 *
 * @return: SUCCESS on success,
 * NULL_POINTER on passing NULL to out_buffer,
 * SYSCALL_ERR if a system call failed,
 * NOT_SUPPORTED if the current environment doesn't support
 * the implemented system calls
 *
 * @note: make sure to pass a pointer that points to at least 37 bytes
 */
util_result_code generate_uuid(char *out_buffer);

/**
 * @brief: Generates a random password
 *
 * @param: length Desired length of the password
 * @param: out_buffer Buffer to store the result
 * @param: flag The charset_flag determine the charachters set
 *
 * @return: SUCCESS on success,
 * INVALID_SIZE on passing invalid length,
 * NULL_POINTER on passing NULL to out_buffer ,
 * SYSCALL_ERR if a system call failed,
 * NOT_SUPPORTED if the current environment doesn't support
 * the implemented system calls
 */
util_result_code generate_password(uint64_t length, char *out_buffer, charset_flag flag);
/**
 * @brief: Copies a string to the Wayland clipboard and schedules an auto-clear.
 *
 * @param: string The null-terminated string to copy.
 *
 * @return: SUCCESS on success,
 * NOT_FOUND if wl-copy tool isn't installed in the environment
 * or the environment isn't on Wayland,
 * SYSCALL_ERR if a system call failed,
 * UNEXPECTED_ERR if an lib function crashes for not knows reason,
 * NOT SUPPORTED if the current environment doesn't support
 * the implemented system calls
 *
 * @note: This function forks a background process that will clear
 * the clipboard after TIMEOUT seconds
 */
util_result_code copy_string_to_clipboard(char *string);

util_result_code random_raw_bytes(uint64_t size, uint8_t *out_buffer);

bool constant_time_equal(const uint8_t *data1, const uint8_t *data2, size_t length);

#endif

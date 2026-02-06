#ifndef TITAN_KEY_SERVICE_H
#define TITAN_KEY_SERVICE_H

#include <stdbool.h>
#include <stdint.h>

/**
 * @file titan_key_service.h
 * @brief Titan Key Management Service
 *
 * Provides secure initialization, validation, loading, and wiping of the Titan Key,
 * which is a cryptographic master key stored on the system. The Titan Key is protected
 * by a MAC for integrity verification.
 *
 * The service uses a versioned blob format with the following structure:
 * - 1 byte: version identifier
 * - 32 bytes: the actual key material
 * - 32 bytes: HMAC for integrity verification
 */

/** @brief Version identifier for Titan Key format v1 */
#define TITAN_KEY_VERSION_01 0x01

/** @brief Size of the key material in bytes (256 bits) */
#define TITAN_KEY_SIZE_V01 32

/** @brief Size of the MAC (HMAC-SHA256) in bytes */
#define TITAN_KEY_MAC_SIZE_V01 32

/** @brief Total blob size: version(1) + key(32) + MAC(32) = 65 bytes */
#define TITAN_BLOB_SIZE_V01 (1 + TITAN_KEY_SIZE_V01 + TITAN_KEY_MAC_SIZE_V01)

/**
 * @brief Return codes for Titan Key Service operations
 *
 * These codes are stored in the global variable tk_status to indicate
 * the result of the last operation performed by the Titan Key Service.
 */
typedef enum {
    /** @brief Operation successful. No errors encountered. */
    TKS_SUCCESS = 0,

    /** @brief Utility error. Hash or cryptographic operation failed.
     *  Stored when: HMAC computation fails during init_titan_key() or load_titan_key() */
    TKS_UTIL_ERR,

    /** @brief Memory/I/O error. Read or write operation failed.
     *  Stored when: write() or read() syscalls fail, indicating file I/O problems */
    TKS_MEM_ERR,

    /** @brief System call error. OS-level operation failed.
     *  Stored when: open(), close(), or stat() syscalls fail with errno set */
    TKS_SYSCALL_ERR,

    /** @brief Unsupported version error. Key blob has unknown version byte.
     *  Stored when: Loaded key file has version != TITAN_KEY_VERSION_01 */
    TKS_UNSUPPORTED_VER_ERR,

    /** @brief Service initialization error. Environment paths setup failed.
     *  Stored when: initialize_paths() fails during init_titan_key() */
    TKS_SERVICE_ERR,

    /** @brief Unsupported operation. Running on non-Linux platform.
     *  Stored when: Platform-specific code (#ifdef __linux__) is not available */
    TKS_UNSOPPORTED_OP,

    /** @brief Key tampered. Data integrity check failed or file permissions compromised.
     *  Stored when: HMAC verification fails, file size mismatch, non-regular file,
     *              or incorrect file permissions (not 0600) in is_valid_titan_key() */
    TKS_TAMPERD,

    /** @brief No Titan Key file found. The key has not been initialized.
     *  Stored when: stat() on key file path fails (ENOENT) in is_exists_titan_key() */
    TKS_NOTKF,

    /** @brief Titan Key file exists. Initialization was already performed.
     *  Stored when: Attempting init_titan_key() when valid key already exists */
    TKS_TKFE
} stk_return_code;

/**
 * @brief Global variable storing the status of the last Titan Key Service operation
 *
 * This variable is updated by every Titan Key Service function to indicate
 * success or the specific type of failure that occurred. Check this variable
 * after calling any titan key function to determine what went wrong.
 *
 * @note This is a global variable for error reporting. Always check tk_status
 *       when a function returns false to understand the reason for failure.
 */
extern stk_return_code tk_status;

/**
 * @brief Initialize the Titan Key by generating and storing a new key
 *
 * Generates a new 256-bit random key and stores it in an encrypted blob
 * with MAC for integrity protection. The blob is written to disk with
 * restricted permissions (0600 - readable/writable by owner only).
 *
 * @return true if initialization successful, false otherwise
 *
 * @post On success: tk_status = TKS_SUCCESS
 * @post On failure: tk_status set to one of:
 *   - TKS_TKFE: Key file already exists and is valid
 *   - TKS_UTIL_ERR: Random number generation or HMAC computation failed
 *   - TKS_SERVICE_ERR: Path initialization failed
 *   - TKS_SYSCALL_ERR: File creation or close() failed
 *   - TKS_UNSOPPORTED_OP: Running on unsupported platform (non-Linux)
 *
 * @note This function uses the system random number generator and requires
 *       cryptographic hashing capability via hash_key()
 */
bool init_titan_key();

/**
 * @brief Load the Titan Key from storage with integrity verification
 *
 * Reads the Titan Key from the stored blob, verifies its integrity using
 * HMAC, and returns the key material if valid.
 *
 * @param[out] out_titan_key Pointer to buffer where key material will be written
 *                            Must be at least TITAN_KEY_SIZE_V01 bytes
 *
 * @return true if key loaded and verified successfully, false otherwise
 *
 * @post On success: tk_status = TKS_SUCCESS, out_titan_key contains 32-byte key
 * @post On failure: tk_status set to one of:
 *   - TKS_NOTKF: Key file does not exist
 *   - TKS_SYSCALL_ERR: File open failed
 *   - TKS_UTIL_ERR: HMAC computation failed during verification
 *   - TKS_TAMPERD: HMAC verification failed (data integrity compromised)
 *   - TKS_UNSUPPORTED_VER_ERR: Key blob has unsupported version byte
 *   - TKS_UNSOPPORTED_OP: Running on unsupported platform (non-Linux)
 *
 * @note This function performs constant-time HMAC comparison to prevent
 *       timing attacks on the MAC verification
 */
bool load_titan_key(uint8_t *out_titan_key);

/**
 * @brief Securely erase the Titan Key from storage
 *
 * Removes the Titan Key file from disk. This operation is irreversible
 * and will prevent any subsequent load operations until a new key is
 * initialized.
 *
 * @return true if key wiped successfully, false otherwise
 *
 * @post On success: tk_status = TKS_SUCCESS, key file deleted from disk
 * @post On failure: tk_status set to one of:
 *   - TKS_NOTKF: Key file does not exist
 *   - TKS_UNSOPPORTED_OP: Running on unsupported platform (non-Linux)
 *
 * @note The unlink() call attempts to securely delete the file,
 *       though filesystem overwriting is not guaranteed
 */
bool wipe_titan_key();

/**
 * @brief Validate the integrity and security of the stored Titan Key
 *
 * Verifies that:
 * - The key file exists
 * - The file size matches the expected blob size (65 bytes)
 * - The file is a regular file (not a directory or device)
 * - The file permissions are correctly restricted (0600)
 *
 * This function does NOT verify the HMAC; use load_titan_key() for
 * complete cryptographic verification.
 *
 * @return true if key file exists and passes validation checks, false otherwise
 *
 * @post On success: tk_status = TKS_SUCCESS
 * @post On failure: tk_status set to one of:
 *   - TKS_NOTKF: Key file does not exist
 *   - TKS_SYSCALL_ERR: stat() call failed
 *   - TKS_TAMPERD: File size mismatch, not a regular file,
 *                  or incorrect permissions
 *   - TKS_UNSOPPORTED_OP: Running on unsupported platform (non-Linux)
 *
 * @note This is a quick validation check. For full security, use
 *       load_titan_key() which performs HMAC verification
 */
bool is_valid_titan_key();

#endif

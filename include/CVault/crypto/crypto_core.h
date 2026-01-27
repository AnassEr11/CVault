#ifndef CRYPTO_CORE_H
#define CRYPTO_CORE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define MAT_KEY_LEN   64
#define TITAN_KEY_LEN 32
#define VER_KEY_LEN   32
#define SALT_LEN      32
#define ARGON_T_COST  3
#define ARGON_M_COST  262144
#define ARGON_P_COST  2
#define IV_LEN        12
#define TAG_LEN       16

/**
 * @brief: hashes a password and a titan_key and a salt and produces a 64 bytes
 * data
 *
 * @param: password a string as const char*
 * @param: titan_key random data of 32 bytes as const uint8_t*
 * @param: salt random data of 32 bytes as const uint8_t*
 * @param: out_key the uint8_t* pointer of which the result will be stored
 *
 * @return: true if succeed, false otherwise
 *
 * @note: consider allocating 64 bytes memory for the out_key pointer
 *
 * @warning: the titan_key and salt pointers must point to 32 bytes memory block
 */
bool derive_key_material(const char *password,
                         const uint8_t *titan_key,
                         const uint8_t *salt,
                         uint8_t *out_key);

/**
 * @brief: hashes a password and a salt and produces a 32 bytes data
 *
 * @param: target the password as a const uint8_t* pointer
 * @param: salt 32 bytes random data as a const uint8_t* pointer
 * @param: out_key the uint8_t* pointer of which the result will be stored
 *
 * @return: true if succeed, false otherwise
 *
 * @note: consider allocating 32 bytes memory for the out_key pointer
 *
 * @warning: the salt pointer must points to 32 bytes memory block
 */
bool hash_key(const uint8_t *key,
			  const uint8_t *salt,
			  uint8_t *out_key);

/**
 * @brief: encrypt data
 *
 * @param: key the master key of which data will be encrypted with, const
 * uint8_t*
 * @param: plaintext the data of which will be encrypted as a const uint8_t*
 * @param: plaintext_len the length of the plaintext as a size_t
 * @param: out_blob the uint8_t* pointer of which the result will be stored
 *
 * @return: true if succeed, false otherwise
 *
 * @note: consider allocating memory for the out_blob pointer
 */
bool encrypt_blob(const uint8_t *key,
                  const uint8_t *plaintext,
                  size_t plaintext_len,
                  uint8_t *out_blob);

/**
 * @brief: decrypt data
 *
 * @param: key the master key of which data will be decrypted with, const
 * uint8_t*
 * @param: blob the data of which will be decrypted as a const uint8_t*
 * @param: blob_len the length of the blob as a size_t
 * @param: out_plaintext the uint8_t* pointer of which the result will be stored
 *
 * @return: true if succeed, false otherwise
 *
 * @note: consider allocating memory for the out_out_plaintext pointer
 */
bool decrypt_blob(const uint8_t *key,
                  const uint8_t *blob,
                  size_t blob_len,
                  uint8_t *out_plaintext);

#endif

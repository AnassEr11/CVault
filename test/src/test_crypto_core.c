#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <CVault/crypto/crypto_core.h>

#define COLOR_RESET "\033[0m"
#define COLOR_GREEN "\033[0;32m"
#define COLOR_RED   "\033[0;31m"
#define COLOR_BLUE  "\033[34m"
#define COLOR_YELLOW "\033[1;33m"

static void print_hex(const char *label, const uint8_t *data, size_t len);

int main() {
	printf(COLOR_BLUE"\nCRYPTO CORE TEST\n"COLOR_GREEN);

    const char *password = "Password123";
    uint8_t titan_key[TITAN_KEY_LEN] = {
        0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
        0x09, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16,
        0x17, 0x18, 0x19, 0x20, 0x21, 0x22, 0x23, 0x24,
        0x25, 0x26, 0x27, 0x28, 0x29, 0x30, 0x31, 0x32
    };
    uint8_t salt[SALT_LEN] = {0xAA, 0xBB, 0xCC};

	printf(COLOR_YELLOW"\n--> setup phase\n\n"COLOR_RESET);

    uint8_t full_material_key[MAT_KEY_LEN];
    if (!derive_key_material(password, titan_key, salt, full_material_key)) {
        fprintf(stderr,COLOR_RED ">> Failed to derive key material\n"COLOR_RESET);
        return 1;
    }
	printf(COLOR_GREEN">> Material key derived successfully\n"COLOR_RESET );
	print_hex("full material key",full_material_key,MAT_KEY_LEN);

    uint8_t master_key[32];
    uint8_t raw_verification_key[32];
    memcpy(master_key, full_material_key, 32);
	printf("\n");
	print_hex("master key", master_key,32);

    memcpy(raw_verification_key, full_material_key + 32, 32);
	printf("\n");
	print_hex("verification key", raw_verification_key,32);

    uint8_t stored_verification_hash[VER_KEY_LEN];
    if (!hash_key(raw_verification_key, salt, stored_verification_hash)) {
        fprintf(stderr, COLOR_RED">> Failed to hash verification key\n"COLOR_RESET);
        return 1;
    }
	printf(COLOR_GREEN">> Verification key hashed successfully\n"COLOR_RESET);
	print_hex("hashed verification key",stored_verification_hash,VER_KEY_LEN);

    const char *plaintext = "Hello, this is a secret message!";
    size_t pt_len = strlen(plaintext) + 1;

    uint8_t blob[pt_len + IV_LEN + TAG_LEN]; 
    if (!encrypt_blob(master_key, (const uint8_t*)plaintext, pt_len, blob)) {
        fprintf(stderr,COLOR_RED ">> Encryption failed\n"COLOR_RESET);
        return 1;
    }
    printf(COLOR_GREEN"\n>> Data encrypted successfully\n"COLOR_RESET);
	print_hex("encrypted blob", blob,pt_len + IV_LEN + TAG_LEN);

    printf(COLOR_YELLOW"\n--> Verification Phase\n\n"COLOR_RESET);
    uint8_t re_derived_material[MAT_KEY_LEN];
    derive_key_material(password, titan_key, salt, re_derived_material);

    uint8_t re_derived_verification[32];
    memcpy(re_derived_verification, re_derived_material + 32, 32);

    uint8_t current_verification_hash[VER_KEY_LEN];
    hash_key(re_derived_verification, salt, current_verification_hash);

    if (memcmp(current_verification_hash, stored_verification_hash, VER_KEY_LEN) == 0) {
        printf(COLOR_GREEN">> Verification Success: Key matches stored hash.\n"COLOR_RESET);

        uint8_t decrypted_text[pt_len];
        if (decrypt_blob(master_key, blob, sizeof(blob), decrypted_text)) {
            printf(COLOR_GREEN"Data decrypted successfully\n"COLOR_RESET);
			printf("resulting data : %s\n\n",(char*)decrypted_text);
            
            if (strcmp(plaintext, (char*)decrypted_text) == 0) {
                printf(COLOR_GREEN">> TEST PASSED: Data integrity verified\n"COLOR_GREEN);
            } else {
                fprintf(stderr,COLOR_RED">> TEST FAILED: Plaintext mismatch\n"COLOR_RESET);
            }
        } else {
            fprintf(stderr,COLOR_RED">> Decryption failed despite key match\n"COLOR_RESET);
        }
    } else {
        fprintf(stderr,COLOR_RED">> Verification FAILED: Key does not match.\n"COLOR_RESET);
    }

    return 0;
}

static void print_hex(const char *label, const uint8_t *data, size_t len) {
    printf("%s: \n", label);
    for (size_t i = 0; i < len; i++) {
        printf("%02X ", data[i]);

        if ((i + 1) % 4 == 0 && (i + 1) % 16 != 0)
            printf(" ");

        if ((i + 1) % 16 == 0)
            printf("\n");
    }
    if (len % 16 != 0)
        printf("\n");
}

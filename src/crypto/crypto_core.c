#include <CVault/crypto/crypto_core.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <stdbool.h>
#include <string.h>
#include <vendor/argon2/argon2.h>

bool derive_key_material(const char *password,const uint8_t *titan_key,
                         const uint8_t *salt,uint8_t *out_key){

    if (!password || !titan_key || !salt || !out_key) {
        return false;
    }

    uint32_t pwd_len = (uint32_t)strlen(password);
    uint32_t titan_len = TITAN_KEY_LEN;

    argon2_context ctx = {.out = out_key,
                          .outlen = MAT_KEY_LEN,
                          .pwd = (uint8_t *)password,
                          .pwdlen = pwd_len,
                          .salt = (uint8_t *)salt,
                          .saltlen = SALT_LEN,
                          .secret = (uint8_t *)titan_key,
                          .secretlen = titan_len,
                          .ad = NULL,
                          .adlen = 0,
                          .t_cost = ARGON_T_COST,
                          .m_cost = ARGON_M_COST,
                          .lanes = ARGON_P_COST,
                          .threads = ARGON_P_COST,
                          .allocate_cbk = NULL,
                          .free_cbk = NULL,
                          .flags = ARGON2_DEFAULT_FLAGS};

    int result = argon2id_ctx(&ctx);

    return (result == ARGON2_OK);
}

bool hash_key(const uint8_t *key, const uint8_t *salt, 
			  uint8_t *out_key){

    if (!key || !salt || !out_key) {
        return false;
    }
	uint32_t out_len = 32;

	int result = argon2id_hash_raw(ARGON_T_COST,
								   ARGON_M_COST,
								   ARGON_P_COST,
								   key,
								   VER_KEY_LEN,
								   salt,
								   SALT_LEN,
								   out_key,
								   out_len);

	return (result == ARGON2_OK);
}

bool encrypt_blob(const uint8_t *key,const uint8_t *plaintext,
				  size_t plaintext_len,uint8_t *out_blob){

    if (!key || !plaintext || !out_blob) {
        return false;
    }

    EVP_CIPHER_CTX *ctx = NULL;
    int len = 0;
    int ciphertext_len = 0;

    uint8_t *iv = out_blob;

    if (RAND_bytes(iv, IV_LEN) != 1){
        return false;
	}

    ctx = EVP_CIPHER_CTX_new();
    if (!ctx){
        return false;
	}

    if (EVP_EncryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, NULL, NULL) != 1){
		EVP_CIPHER_CTX_free(ctx);
		return false;
	}

    if (EVP_EncryptInit_ex(ctx, NULL, NULL, key, iv) != 1){
		EVP_CIPHER_CTX_free(ctx);
		return false;
	}

    uint8_t *ciphertext = out_blob + IV_LEN;

    if (EVP_EncryptUpdate(ctx,ciphertext,&len,
						  plaintext,(int)plaintext_len) != 1){
		EVP_CIPHER_CTX_free(ctx);
		return false;
	}

    ciphertext_len = len;

    if (EVP_EncryptFinal_ex(ctx, ciphertext + len, &len) != 1){
		EVP_CIPHER_CTX_free(ctx);
		return false;
	}


    ciphertext_len += len;

    uint8_t *tag = ciphertext + ciphertext_len;

    if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG, TAG_LEN, tag) != 1){
		EVP_CIPHER_CTX_free(ctx);
		return false;
	}

    EVP_CIPHER_CTX_free(ctx);
    return true;

}

bool decrypt_blob(const uint8_t *key,const uint8_t *blob,
                  size_t blob_len,uint8_t *out_plaintext){

    if (!key || !blob || !out_plaintext) {
        return false;
    }

    if (blob_len < IV_LEN + TAG_LEN) {
        return false;
    }

    size_t ciphertext_len = blob_len - IV_LEN - TAG_LEN;

    const uint8_t *iv = blob;
    const uint8_t *ciphertext = blob + IV_LEN;
    const uint8_t *tag = blob + IV_LEN + ciphertext_len;

    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx){
        return false;
	}

    int len = 0;

    if (EVP_DecryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, NULL, NULL) != 1) {
		EVP_CIPHER_CTX_free(ctx);
		return false;
    }

    if (EVP_DecryptInit_ex(ctx, NULL, NULL, key, iv) != 1) {
		EVP_CIPHER_CTX_free(ctx);
		return false;
    }

    if (EVP_DecryptUpdate(ctx,out_plaintext,&len,
						  ciphertext,(int)ciphertext_len) != 1) {
		EVP_CIPHER_CTX_free(ctx);
		return false;
    }

    if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_TAG,
							TAG_LEN, (void *)tag) != 1) {
		EVP_CIPHER_CTX_free(ctx);
		return false;
    }

    if (EVP_DecryptFinal_ex(ctx, out_plaintext + len, &len) != 1) {
		EVP_CIPHER_CTX_free(ctx);
		return false;
    }

    EVP_CIPHER_CTX_free(ctx);
    return true;

}

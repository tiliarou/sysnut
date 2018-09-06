#pragma once
#include "nx/buffer.h"
#include "mbedtls/cipher.h"

typedef enum {
	CRYPT_NULL = 0,
	CRYPT_NONE = 1,
	CRYPT_XTS = 2,
	CRYPT_CTR = 3,
	CRYPT_BKTR = 4,
	CRYPT_NCA0 = 0x3041434E //MAGIC_NCA0
} crypt_type_t;

typedef mbedtls_cipher_type_t aes_mode_t;


class Crypto
{
public:
	Crypto();
	Crypto(const void *key, unsigned int key_size, aes_mode_t mode);
	~Crypto();

	void setIv(const void *iv, size_t l);

	void encrypt(void *dst, const void *src, size_t l);
	void decrypt(void *dst, const void *src, size_t l);

	void calculateCmac(void *dst, void *src, size_t size, const void *key);

	void xtsEncrypt(void *dst, const void *src, size_t l, size_t sector, size_t sector_size);
	void xtsDecrypt(void *dst, const void *src, size_t l, size_t sector, size_t sector_size);

private:
	mbedtls_cipher_context_t cipherEnc;
	mbedtls_cipher_context_t cipherDec;
};


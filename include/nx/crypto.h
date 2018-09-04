#pragma once

#include "mbedtls/cipher.h"

#define HEADER_KEY "\xAE\xAA\xB1\xCA\x08\xAD\xF9\xBE\xF1\x29\x91\xF3\x69\xE3\xC5\x67\xD6\x88\x1E\x4E\x4A\x6A\x47\xA5\x1F\x6E\x48\x77\x06\x2D\x54\x2D"
#define HEADER_KEY2 "\x2D\x54\x2D\x06\x77\x48\x6E\x1F\xA5\x47\x6A\x4A\x4E\x1E\x88\xD6\x67\xC5\xE3\x69\xF3\x91\x29\xF1\xBE\xF9\xAD\x08\xCA\xB1\xAA\xAE"

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


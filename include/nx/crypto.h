#pragma once
#include "nx/buffer.h"

#include "mbedtls/cipher.h"

#define HEADER_KEY "AEAAB1CA08ADF9BEF12991F369E3C567D6881E4E4A6A47A51F6E4877062D542D"

typedef mbedtls_cipher_type_t aes_mode_t;

Buffer uhx(const char* hex, int len = 0);

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


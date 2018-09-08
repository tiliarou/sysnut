#pragma once
#include "nx/primitives.h"
#include "nx/buffer.h"
#include "mbedtls/cipher.h"

typedef enum : u8 {
	CRYPT_NULL = 0,
	CRYPT_NONE = 1,
	CRYPT_XTS = 2,
	CRYPT_CTR = 3,
	CRYPT_BKTR = 4
} crypt_type_t;

typedef mbedtls_cipher_type_t aes_mode_t;


class Crypto
{
public:
	Crypto();
	Crypto(const void *key, unsigned int key_size, aes_mode_t mode);
	~Crypto();

	bool setIv(const void *iv, size_t l);
	bool setKey(const void *key, u64 key_size);
	bool setMode(aes_mode_t mode);
	bool setCounter(const u8* counter, u32 sz);

	void encrypt(void *dst, const void *src, size_t l);
	void decrypt(void *dst, const void *src, size_t l);

	void calculateCmac(void *dst, void *src, size_t size, const void *key);

	void xtsEncrypt(void *dst, const void *src, size_t l, size_t sector, size_t sector_size);
	void xtsDecrypt(void *dst, const void *src, size_t l, size_t sector, size_t sector_size);

	const u8* updateCounter(u64 ofs);

private:
	u8 counter[0x10];
	mbedtls_cipher_context_t cipherEnc;
	mbedtls_cipher_context_t cipherDec;
};


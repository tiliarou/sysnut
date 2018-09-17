#pragma once
#include "nx/primitives.h"
#include "nx/buffer.h"
#include "mbedtls/cipher.h"
#include "mbedtls/sha256.h"

typedef enum : u8 {
	CRYPT_NULL = 0,
	CRYPT_NONE = 1,
	CRYPT_XTS = 2,
	CRYPT_CTR = 3,
	CRYPT_BKTR = 4
} crypt_type_t;

typedef mbedtls_cipher_type_t aes_mode_t;

class AesCtr
{
public:
	AesCtr() : m_high(0), m_low(0)
	{
	}

	AesCtr(u64 iv) : m_high(swapEndian(iv)), m_low(0)
	{
	}

	u64& high() { return m_high; }
	u64& low() { return m_low; }
private:
	u64 m_high;
	u64 m_low;
};


class Crypto
{
public:
	Crypto();
	Crypto(const void *key, unsigned int key_size, aes_mode_t mode);
	~Crypto();

	bool setIv(const void *iv, size_t l);
	bool setKey(const void *key, u64 key_size);
	bool setMode(aes_mode_t mode);
	bool setCounter(const AesCtr counter);

	void encrypt(void *dst, const void *src, size_t l);
	void decrypt(void *dst, const void *src, size_t l);

	void xtsEncrypt(void *dst, const void *src, size_t l, size_t sector, size_t sector_size);
	void xtsDecrypt(void *dst, const void *src, size_t l, size_t sector, size_t sector_size);

	const AesCtr& updateCounter(u64 ofs);

private:
	AesCtr counter;
	mbedtls_cipher_context_t cipherEnc;
	mbedtls_cipher_context_t cipherDec;
};

template<class T>
integer<256> sha256(T n)
{
	integer<256> result(0);

	mbedtls_sha256_context ctx;
	mbedtls_sha256_init(&ctx);

	mbedtls_sha256_update(&ctx, (const u8*)&n, sizeof(T));

	mbedtls_sha256_finish(&ctx, (u8*)&result);

	mbedtls_sha256_free(&ctx);
	return result;
}

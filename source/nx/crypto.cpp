#include <stdlib.h>
#include <stdio.h>
#include "nx/crypto.h"
#include "log.h"

Crypto::Crypto()
{
}

Crypto::Crypto(const void *key, unsigned int key_size, aes_mode_t mode)
{
    mbedtls_cipher_init(&cipherDec);
    mbedtls_cipher_init(&cipherEnc);
    
    if (mbedtls_cipher_setup(&cipherDec, mbedtls_cipher_info_from_type(mode)) || mbedtls_cipher_setup(&cipherEnc, mbedtls_cipher_info_from_type(mode)))
	{
        fatal("Failed to set up AES context!\n");
    }
        
    if (mbedtls_cipher_setkey(&cipherDec, (const unsigned char*)key, key_size * 8, MBEDTLS_ENCRYPT) || mbedtls_cipher_setkey(&cipherEnc, (const unsigned char*)key, key_size * 8, MBEDTLS_ENCRYPT))
	{
        fatal("Failed to set key for AES context!\n");
    }
}


Crypto::~Crypto()
{    
    mbedtls_cipher_free(&cipherDec);
    mbedtls_cipher_free(&cipherEnc);
}

void Crypto::setIv(const void *iv, size_t l)
{
    if (mbedtls_cipher_set_iv(&cipherDec, (const unsigned char*)iv, l) || mbedtls_cipher_set_iv(&cipherEnc, (const unsigned char*)iv, l))
	{
        fatal("Failed to set IV for AES context!\n");
    }
}

/* Calculate CMAC. */
void Crypto::calculateCmac(void *dst, void *src, size_t size, const void *key) {
    /*mbedtls_cipher_context_t m_ctx;
    mbedtls_cipher_init(&m_ctx);

    if (mbedtls_cipher_setup(&m_ctx, mbedtls_cipher_info_from_type(MBEDTLS_CIPHER_AES_128_ECB)) || mbedtls_cipher_cmac_starts(&m_ctx, key, 0x80) || mbedtls_cipher_cmac_update(&m_ctx, src, size) || mbedtls_cipher_cmac_finish(&m_ctx, dst))
	{
            fatal("Failed to calculate CMAC!\n");
    }*/
}


/* Encrypt with context. */
void Crypto::encrypt(void *dst, const void *src, size_t l)
{
    size_t out_len = 0;
    
    /* Prepare context */
    mbedtls_cipher_reset(&cipherEnc);
    
    /* XTS doesn't need per-block updating */
	if (mbedtls_cipher_get_cipher_mode(&cipherEnc) == MBEDTLS_MODE_XTS)
	{
		mbedtls_cipher_update(&cipherEnc, (const unsigned char *)src, l, (unsigned char *)dst, &out_len);
	}
    else
    {
        unsigned int blk_size = mbedtls_cipher_get_block_size(&cipherEnc);
        
        /* Do per-block updating */
        for (int offset = 0; (unsigned int)offset < l; offset += blk_size)
        {
            int len = ((unsigned int)(l - offset) > blk_size) ? blk_size : (unsigned int) (l - offset);
            mbedtls_cipher_update(&cipherEnc, (const unsigned char * )src + offset, len, (unsigned char *)dst + offset, &out_len);
        }
    }
    
    /* Flush all data */
    mbedtls_cipher_finish(&cipherEnc, NULL, NULL);
}

/* Decrypt with context. */
void Crypto::decrypt(void *dst, const void *src, size_t l)
{
    size_t out_len = 0;
    
    /* Prepare context */
    mbedtls_cipher_reset(&cipherDec);
    
    /* XTS doesn't need per-block updating */
	if (mbedtls_cipher_get_cipher_mode(&cipherDec) == MBEDTLS_MODE_XTS)
	{
		mbedtls_cipher_update(&cipherDec, (const unsigned char *)src, l, (unsigned char *)dst, &out_len);
	}
    else
    {
        unsigned int blk_size = mbedtls_cipher_get_block_size(&cipherDec);
        
        /* Do per-block updating */
        for (int offset = 0; (unsigned int)offset < l; offset += blk_size)
        {
            int len = ((unsigned int)(l - offset) > blk_size) ? blk_size : (unsigned int) (l - offset);
            mbedtls_cipher_update(&cipherDec, (const unsigned char * )src + offset, len, (unsigned char *)dst + offset, &out_len);
        }
    }
    
    /* Flush all data */
    mbedtls_cipher_finish(&cipherDec, NULL, NULL);
}

static void getTweak(unsigned char *tweak, size_t sector) {
    for (int i = 0xF; i >= 0; i--) { /* Nintendo LE custom tweak... */
        tweak[i] = (unsigned char)(sector & 0xFF);
        sector >>= 8;
    }
}

/* Encrypt with context for XTS. */
void Crypto::xtsEncrypt(void *dst, const void *src, size_t l, size_t sector, size_t sector_size) {
    unsigned char tweak[0x10];

    if (l % sector_size != 0)
	{
        fatal("Length must be multiple of sectors!\n");
		return;
    }

    for (size_t i = 0; i < l; i += sector_size) {
        /* Workaround for Nintendo's custom sector...manually generate the tweak. */
        getTweak(tweak, sector++);
        setIv(tweak, 16);
        encrypt((char *)dst + i, (const char *)src + i, sector_size);
    }
}

/* Decrypt with context for XTS. */
void Crypto::xtsDecrypt( void *dst, const void *src, size_t l, size_t sector, size_t sector_size) {
    unsigned char tweak[0x10];

    if (l % sector_size != 0)
	{
        fatal("Length must be multiple of sectors!\n");
		return;
    }

    for (size_t i = 0; i < l; i += sector_size) {
        /* Workaround for Nintendo's custom sector...manually generate the tweak. */
        getTweak(tweak, sector++);
        setIv(tweak, 16);
        decrypt((char *)dst + i, (const char *)src + i, sector_size);
    }
}

static int ishex(char c) {
	if ('a' <= c && c <= 'f') return 1;
	if ('A' <= c && c <= 'F') return 1;
	if ('0' <= c && c <= '9') return 1;
	return 0;
}

static char hextoi(char c) {
	if ('a' <= c && c <= 'f') return c - 'a' + 0xA;
	if ('A' <= c && c <= 'F') return c - 'A' + 0xA;
	if ('0' <= c && c <= '9') return c - '0';
	return 0;
}

Buffer uhx(const char* hex, int len)
{
	Buffer buf;

	if (len)
	{
		len *= 2;
	}
	else
	{
		len = strlen(hex);
	}

	buf.resize(len / 2);

	if (len % 2)
	{
		error("Key (%s) must be an even number of characters!\n", hex, len);
		buf.resize(0);
		return buf;
	}

	for (unsigned int i = 0; i < len / 2; i++)
	{
		if (!ishex(hex[i]))
		{
			error("Key (%s) must be %d hex digits!\n", hex, len);
			buf.resize(0);
			return buf;
		}
	}

	u8* key = (u8*)buf.buffer();
	memset(key, 0, len >> 1);

	for (unsigned int i = 0; i < len; i++) {
		char val = hextoi(hex[i]);

		if ((i & 1) == 0) {
			val <<= 4;
		}
		key[i >> 1] |= val;
	}

	return buf;
}
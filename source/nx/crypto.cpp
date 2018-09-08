#include <stdlib.h>
#include <stdio.h>
#include "nx/crypto.h"
#include "log.h"

Crypto::Crypto()
{
	mbedtls_cipher_init(&cipherDec);
	mbedtls_cipher_init(&cipherEnc);
}

Crypto::Crypto(const void *key, unsigned int key_size, aes_mode_t mode)
{
    mbedtls_cipher_init(&cipherDec);
    mbedtls_cipher_init(&cipherEnc);
    
	setMode(mode);
	setKey(key, key_size);
}

bool Crypto::setKey(const void *key, u64 key_size)
{
	if (mbedtls_cipher_setkey(&cipherDec, (const unsigned char*)key, int(key_size * 8), MBEDTLS_DECRYPT) || mbedtls_cipher_setkey(&cipherEnc, (const unsigned char*)key, int(key_size * 8), MBEDTLS_ENCRYPT))
	{
		fatal("Failed to set key for AES context!\n");
		return false;
	}
	return true;
}

bool Crypto::setMode(aes_mode_t mode)
{
	if (mbedtls_cipher_setup(&cipherDec, mbedtls_cipher_info_from_type(mode)) || mbedtls_cipher_setup(&cipherEnc, mbedtls_cipher_info_from_type(mode)))
	{
		fatal("Failed to set key for AES context!\n");
		return false;
	}
	return true;
}

bool Crypto::setCounter(const u8* counter, u32 sz)
{
	memset(this->counter, 0, sizeof(this->counter));
	memcpy(this->counter, counter, sz);
	return setIv(this->counter, sizeof(this->counter));
}


Crypto::~Crypto()
{    
    mbedtls_cipher_free(&cipherDec);
    mbedtls_cipher_free(&cipherEnc);
}

bool Crypto::setIv(const void *iv, size_t l)
{
	int r1=0, r2 = 0;
    if ((r1=mbedtls_cipher_set_iv(&cipherDec, (const unsigned char*)iv, l)) || (r2=mbedtls_cipher_set_iv(&cipherEnc, (const unsigned char*)iv, l)))
	{
		fatal("Failed to set IV for AES context! %d, %d, %x, %d, %x, %x\n", r1, r2, iv, l, &cipherEnc, cipherEnc.cipher_info->iv_size);
		return false;
    }
	return true;
}

const u8* Crypto::updateCounter(u64 ofs)
{
	ofs >>= 4;
	for (unsigned int j = 0; j < 0x8; j++) {
		counter[0x10 - j - 1] = (unsigned char)(ofs & 0xFF);
		ofs >>= 8;
	}
	setIv(counter, sizeof(counter));
	return counter;
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

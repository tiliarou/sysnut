#include "nx/keys.h"
#include <string.h>
#include <stdio.h>
#include "nx/crypto.h"
#include "nx/string.h"
#include "log.h"

#ifdef _MSC_VER
Keys g_keys("keys.txt");
#else
Keys g_keys("/switch/deez/keys.txt");
#endif

#define VERIFY_KEY(NAME, KEY, HASH) \
if (sha256(KEY) != integer<256>(HASH)) \
{ \
	print("Bad key hash: %s = %s, expected %s\n", NAME, hx(sha256(KEY)).c_str(), hx(integer<256>(HASH)).c_str()); \
}

Keys& keys()
{
	return g_keys;
}

void generateKek(integer<128>& dst, const integer<128> src, const integer<128> masterKey, const integer<128> kekSeed, const integer<128> keySeed)
{
	integer<128> kek;
	integer<128> srcKek;

	{
		Crypto crypto(&masterKey, sizeof(masterKey), MBEDTLS_CIPHER_AES_128_ECB);
		crypto.decrypt(&kek, &kekSeed, sizeof(kekSeed));
	}

	{
		Crypto crypto(&kek, sizeof(kek), MBEDTLS_CIPHER_AES_128_ECB);
		crypto.decrypt(&srcKek, &src, sizeof(src));
	}

	if (keySeed == integer<128>(0))
	{
		memcpy(&dst, &srcKek, sizeof(srcKek));
	}
	else
	{
		Crypto crypto(&srcKek, sizeof(srcKek), MBEDTLS_CIPHER_AES_128_ECB);
		crypto.decrypt(&dst, &keySeed, sizeof(keySeed));
	}
}

static int getKv(FILE *f, char **key, char **value)
{
#define SKIP_SPACE(p)   do {\
    for (; *p == ' ' || *p == '\t'; ++p)\
        ;\
} while(0);
	static char line[1024];
	char *k, *v, *p, *end;

	*key = *value = NULL;

	errno = 0;
	if (fgets(line, (int)sizeof(line), f) == NULL) {
		if (feof(f))
			return 1;
		else
			return -2;
	}
	if (errno != 0)
		return -2;

	if (*line == '\n' || *line == '\r' || *line == '\0')
		return 0;

	/* Not finding \r or \n is not a problem.
	* The line might just be exactly 512 characters long, we have no way to
	* tell.
	* Additionally, it's possible that the last line of a file is not actually
	* a line (i.e., does not end in '\n'); we do want to handle those.
	*/
	if ((p = strchr(line, '\r')) != NULL || (p = strchr(line, '\n')) != NULL) {
		end = p;
		*p = '\0';
	}
	else {
		end = line + strlen(line) + 1;
	}

	p = line;
	SKIP_SPACE(p);
	k = p;

	/* Validate key and convert to lower case. */
	for (; *p != ' ' && *p != ',' && *p != '\t' && *p != '='; ++p) {
		if (*p == '\0')
			return -1;

		if (*p >= 'A' && *p <= 'Z') {
			*p = 'a' + (*p - 'A');
			continue;
		}

		if (*p != '_' &&
			(*p < '0' || *p > '9') &&
			(*p < 'a' || *p > 'z')) {
			return -1;
		}
	}

	/* Bail if the final ++p put us at the end of string */
	if (*p == '\0')
		return -1;

	/* We should be at the end of key now and either whitespace or [,=]
	* follows.
	*/
	if (*p == '=' || *p == ',') {
		*p++ = '\0';
	}
	else {
		*p++ = '\0';
		SKIP_SPACE(p);
		if (*p != '=' && *p != ',')
			return -1;
		*p++ = '\0';
	}

	/* Empty key is an error. */
	if (*k == '\0')
		return -1;

	SKIP_SPACE(p);
	v = p;

	/* Skip trailing whitespace */
	for (p = end - 1; *p == '\t' || *p == ' '; --p)
		;

	*(p + 1) = '\0';

	*key = k;
	*value = v;

	return 0;
#undef SKIP_SPACE
}

template<class T>
T* uhx(T& key, const char* hex, u64 len)
{

	if (len)
	{
		len *= 2;
	}
	else
	{
		len = sizeof(key);
	}

	if (len % 2)
	{
		error("Key (%s) must be an even number of characters!\n", hex, len);
		return NULL;
	}

	for (unsigned int i = 0; i < len / 2; i++)
	{
		if (!ishex(hex[i]))
		{
			error("Key (%s) must be %d hex digits!\n", hex, len);
			return NULL;
		}
	}

	memset(key, 0, (size_t)(len >> 1));
	u8* ptr = reinterpret_cast<u8*>(&key);

	for (unsigned int i = 0; i < len; i++) {
		char val = hextoi(hex[i]);

		if ((i & 1) == 0) {
			val <<= 4;
		}
		ptr[i >> 1] |= val;
	}

	return (T*)&key;
}

Buffer<u8> uhx(const char* hex, u64 len)
{
	Buffer<u8> buf;

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
	memset(key, 0, (size_t)(len >> 1));

	for (unsigned int i = 0; i < len; i++) {
		char val = hextoi(hex[i]);

		if ((i & 1) == 0) {
			val <<= 4;
		}
		key[i >> 1] |= val;
	}

	return buf;
}

Keys::Keys()
{
}

Keys::Keys(const char* fileName)
{
	open(fileName);
}

bool Keys::open(const char* file)
{
	char *key, *value;
	int ret;

	FILE* f = fopen(file, "r");

	if (!f)
	{
		fatal("Failed to open key file for reading! %s\n", file);
		return false;
	}

	while ((ret = getKv(f, &key, &value)) != 1 && ret != -2)
	{
		if (ret == 0) {
			if (key == NULL || value == NULL) {
				continue;
			}
			int matchedKey = 0;
			if (strcmp(key, "aes_kek_generation_source") == 0) {
				uhx(aesKekGenerationSource, value, sizeof(aesKekGenerationSource));
				matchedKey = 1;
			}
			else if (strcmp(key, "aes_key_generation_source") == 0) {
				uhx(aesKeyGenerationSource, value, sizeof(aesKeyGenerationSource));
				matchedKey = 1;
			}
			else if (strcmp(key, "key_area_key_application_source") == 0) {
				uhx(keyAreaKeyApplicationSource, value, sizeof(keyAreaKeyApplicationSource));
				matchedKey = 1;
			}
			else if (strcmp(key, "key_area_key_ocean_source") == 0) {
				uhx(keyAreaKeyOceanSource, value, sizeof(keyAreaKeyOceanSource));
				matchedKey = 1;
			}
			else if (strcmp(key, "key_area_key_system_source") == 0) {
				uhx(keyAreaKeySystemSource, value, sizeof(keyAreaKeySystemSource));
				matchedKey = 1;
			}
			else if (strcmp(key, "titlekek_source") == 0) {
				uhx(titlekekSource, value, sizeof(titlekekSource));
				VERIFY_KEY("titlekek_source", titlekekSource, "445e3495308d9ef16f215997cce027419adaaca5661e933e5834cc4f5d8bfaf2");
				matchedKey = 1;
			}
			else if (strcmp(key, "header_kek_source") == 0) {
				uhx(headerKekSource, value, sizeof(headerKekSource));
				VERIFY_KEY("header_kek_source", headerKekSource, "b97b807cef362d3ab4529c27685ed0e3b64151f20a377c62607ef83e81a20a07");
				matchedKey = 1;
			}
			else if (strcmp(key, "header_key_source") == 0) {
				uhx(headerKeySource, value, sizeof(headerKeySource));
				VERIFY_KEY("header_key_source", headerKeySource, "fd03479aecb42b9d642c377925022cd0ba6ca3a7deffbbad7c718e7ad6b585ae");
				matchedKey = 1;
			}
			else if (strcmp(key, "header_key") == 0) {
				uhx(headerKey, value, sizeof(headerKey));
				VERIFY_KEY("header_key", headerKey, "52aa1da1624d2945e74d2318b7cc386769a66c96c4d7f7e8292eeca6d123203d");
				matchedKey = 1;
			}
			else if (strcmp(key, "package2_key_source") == 0) {
				uhx(package2KeySource, value, sizeof(package2KeySource));
				matchedKey = 1;
			}
			else if (strcmp(key, "sd_card_kek_source") == 0) {
				uhx(sdCardKekSource, value, sizeof(sdCardKekSource));
				VERIFY_KEY("sd_card_kek_source", sdCardKekSource, "830295736c5e3579c75cd71a6cdb630e8e4b490672219f5377dc731b6e5671f1");
				matchedKey = 1;
			}
			else if (strcmp(key, "sd_card_nca_key_source") == 0) {
				uhx(sdCardKeySources[1], value, sizeof(sdCardKeySources[1]));
				VERIFY_KEY("sd_card_nca_key_source", sdCardKeySources[1], "699b53eb637b6b14e8fadfe5a05dcf79b447e84e7c6562ce2eb3b2c6d8a3db51");
				matchedKey = 1;
			}
			else if (strcmp(key, "sd_card_save_key_source") == 0) {
				uhx(sdCardKeySources[0], value, sizeof(sdCardKeySources[0]));
				VERIFY_KEY("sd_card_save_key_source", sdCardKeySources[0], "f45c8cc44a91bb5ea56d896b22fbd84d1fb0241fb736ebdc4085921b61497634");
				matchedKey = 1;
			}
			else if (strcmp(key, "master_key_source") == 0) {
				uhx(masterKeySource, value, sizeof(masterKeySource));
				matchedKey = 1;
			}
			else if (strcmp(key, "keyblob_mac_key_source") == 0) {
				uhx(keyblobMacKeySource, value, sizeof(keyblobMacKeySource));
				matchedKey = 1;
			}
			else if (strcmp(key, "secure_boot_key") == 0) {
				uhx(secureBootKey, value, sizeof(secureBootKey));
				matchedKey = 1;
			}
			else if (strcmp(key, "tsec_key") == 0) {
				uhx(tsecKey, value, sizeof(tsecKey));
				matchedKey = 1;
			}
			/*else if (strcmp(key, "betaNca0Exponent") == 0) {
				unsigned char exponent[0x100] = { 0 };
				uhx(exponent, value, sizeof(exponent));
				pki_set_beta_nca0_exponent(exponent);
				matchedKey = 1;
			}*/
			else {
				char testName[0x100] = { 0 };
				for (unsigned int i = 0; i < 0x20 && !matchedKey; i++) {
					snprintf(testName, sizeof(testName), "keyblob_key_source_%02x", i);
					if (strcmp(key, testName) == 0) {
						uhx(keyblobKeySources[i], value, sizeof(keyblobKeySources[i]));
						matchedKey = 1;
						break;
					}

					snprintf(testName, sizeof(testName), "keyblob_key_%02x", i);
					if (strcmp(key, testName) == 0) {
						uhx(keyblobKeys[i], value, sizeof(keyblobKeys[i]));
						matchedKey = 1;
						break;
					}

					snprintf(testName, sizeof(testName), "keyblob_mac_key_%02x", i);
					if (strcmp(key, testName) == 0) {
						uhx(keyblobMacKeys[i], value, sizeof(keyblobMacKeys[i]));
						matchedKey = 1;
						break;
					}

					snprintf(testName, sizeof(testName), "encrypted_keyblob_%02x", i);
					if (strcmp(key, testName) == 0) {
						uhx(encryptedKeyBlobs[i], value, sizeof(encryptedKeyBlobs[i]));
						matchedKey = 1;
						break;
					}

					snprintf(testName, sizeof(testName), "keyblob_%02x", i);
					if (strcmp(key, testName) == 0) {
						uhx(keyblobs[i], value, sizeof(keyblobs[i]));
						matchedKey = 1;
						break;
					}

					snprintf(testName, sizeof(testName), "master_key_%02x", i);
					if (strcmp(key, testName) == 0) {
						uhx(masterKeys[i], value, sizeof(masterKeys[i]));
						switch (i)
						{
							case 0:
								VERIFY_KEY(testName, masterKeys[i], "dc48df7ca2548457750c0f1178f38d0b719046979b60a6298be43f70b0c34b82");
								break;
							case 1:
								VERIFY_KEY(testName, masterKeys[i], "4165c3cd4a108f521e66b461c74e77c7386ef2160174bd556982d6e8a7311236");
								break;
							case 2:
								VERIFY_KEY(testName, masterKeys[i], "050fb20068c3b92e6bae48d8d89fb9ccf6eedd72ab8e2f8a814081af7f5796e8");
								break;
							case 3:
								VERIFY_KEY(testName, masterKeys[i], "995042dc56a3daad15e1cc9b1e85b04adef2ffdf1478288ad15781138c529420");
								break;
							case 4:
								VERIFY_KEY(testName, masterKeys[i], "2b454c48b6aa2a4f8a15b11df367422ee2b512aa9a0b10f70dacdb47bd3fa9a6");
								break;
							default:
								VERIFY_KEY(testName, masterKeys[i], "0000000000000000000000000000000000000000000000000000000000000000");
								break;
						}
						matchedKey = 1;

						break;
					}

					snprintf(testName, sizeof(testName), "package1_key_%02x", i);
					if (strcmp(key, testName) == 0) {
						uhx(package1Keys[i], value, sizeof(package1Keys[i]));
						matchedKey = 1;
						break;
					}

					snprintf(testName, sizeof(testName), "package2_key_%02x", i);
					if (strcmp(key, testName) == 0) {
						uhx(package2Keys[i], value, sizeof(package2Keys[i]));
						matchedKey = 1;
						break;
					}

					snprintf(testName, sizeof(testName), "titlekek_%02x", i);
					if (strcmp(key, testName) == 0) {
						uhx(titlekeks[i], value, sizeof(titlekeks[i]));
						matchedKey = 1;
						break;
					}

					snprintf(testName, sizeof(testName), "key_area_key_application_%02x", i);
					if (strcmp(key, testName) == 0) {
						uhx(m_keyAreaKeys[i][0], value, sizeof(m_keyAreaKeys[i][0]));
						matchedKey = 1;
						break;
					}

					snprintf(testName, sizeof(testName), "key_area_key_ocean_%02x", i);
					if (strcmp(key, testName) == 0) {
						uhx(m_keyAreaKeys[i][1], value, sizeof(m_keyAreaKeys[i][1]));
						matchedKey = 1;
						break;
					}

					snprintf(testName, sizeof(testName), "key_area_key_system_%02x", i);
					if (strcmp(key, testName) == 0) {
						uhx(m_keyAreaKeys[i][2], value, sizeof(m_keyAreaKeys[i][2]));
						matchedKey = 1;
						break;
					}
				}
			}
			if (!matchedKey) {
				fprintf(stderr, "[WARN]: Failed to match key \"%s\", (value \"%s\")\n", key, value);
			}
		}
	}

	fclose(f);

	if (masterKeys[0].empty())
	{
		error("Missing masterKey0 key!\n");
	}

	if (titlekekSource.empty())
	{
		error("Missing titleKekSource key!\n");
	}
	return true;
}
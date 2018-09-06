#include "nx/keys.h"
#include <string.h>
#include <stdio.h>
#include "log.h"

Keys keys("keys.txt");

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

u8* uhx(u8* key, const char* hex, int len)
{

	if (len)
	{
		len *= 2;
	}
	else
	{
		len = strlen(hex);
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

	memset(key, 0, len >> 1);

	for (unsigned int i = 0; i < len; i++) {
		char val = hextoi(hex[i]);

		if ((i & 1) == 0) {
			val <<= 4;
		}
		key[i >> 1] |= val;
	}

	return key;
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
			if (strcmp(key, "aesKekGenerationSource") == 0) {
				uhx(aesKekGenerationSource, value, sizeof(aesKekGenerationSource));
				matchedKey = 1;
			}
			else if (strcmp(key, "aesKeyGenerationSource") == 0) {
				uhx(aesKeyGenerationSource, value, sizeof(aesKeyGenerationSource));
				matchedKey = 1;
			}
			else if (strcmp(key, "keyAreaKeyApplicationSource") == 0) {
				uhx(keyAreaKeyApplicationSource, value, sizeof(keyAreaKeyApplicationSource));
				matchedKey = 1;
			}
			else if (strcmp(key, "keyAreaKeyOceanSource") == 0) {
				uhx(keyAreaKeyOceanSource, value, sizeof(keyAreaKeyOceanSource));
				matchedKey = 1;
			}
			else if (strcmp(key, "keyAreaKeySystemSource") == 0) {
				uhx(keyAreaKeySystemSource, value, sizeof(keyAreaKeySystemSource));
				matchedKey = 1;
			}
			else if (strcmp(key, "titlekekSource") == 0) {
				uhx(titlekekSource, value, sizeof(titlekekSource));
				matchedKey = 1;
			}
			else if (strcmp(key, "headerKekSource") == 0) {
				uhx(headerKekSource, value, sizeof(headerKekSource));
				matchedKey = 1;
			}
			else if (strcmp(key, "headerKeySource") == 0) {
				uhx(headerKeySource, value, sizeof(headerKeySource));
				matchedKey = 1;
			}
			else if (strcmp(key, "headerKey") == 0) {
				uhx(headerKey, value, sizeof(headerKey));
				matchedKey = 1;
			}
			else if (strcmp(key, "package2KeySource") == 0) {
				uhx(package2KeySource, value, sizeof(package2KeySource));
				matchedKey = 1;
			}
			else if (strcmp(key, "sdCardKekSource") == 0) {
				uhx(sdCardKekSource, value, sizeof(sdCardKekSource));
				matchedKey = 1;
			}
			else if (strcmp(key, "sdCardNcaKeySource") == 0) {
				uhx(sdCardKeySources[1], value, sizeof(sdCardKeySources[1]));
				matchedKey = 1;
			}
			else if (strcmp(key, "sdCardSaveKeySource") == 0) {
				uhx(sdCardKeySources[0], value, sizeof(sdCardKeySources[0]));
				matchedKey = 1;
			}
			else if (strcmp(key, "masterKeySource") == 0) {
				uhx(masterKeySource, value, sizeof(masterKeySource));
				matchedKey = 1;
			}
			else if (strcmp(key, "keyblobMacKeySource") == 0) {
				uhx(keyblobMacKeySource, value, sizeof(keyblobMacKeySource));
				matchedKey = 1;
			}
			else if (strcmp(key, "secureBootKey") == 0) {
				uhx(secureBootKey, value, sizeof(secureBootKey));
				matchedKey = 1;
			}
			else if (strcmp(key, "tsecKey") == 0) {
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
					snprintf(testName, sizeof(testName), "keyblobKeySource_%02x", i);
					if (strcmp(key, testName) == 0) {
						uhx(keyblobKeySources[i], value, sizeof(keyblobKeySources[i]));
						matchedKey = 1;
						break;
					}

					snprintf(testName, sizeof(testName), "keyblobKey_%02x", i);
					if (strcmp(key, testName) == 0) {
						uhx(keyblobKeys[i], value, sizeof(keyblobKeys[i]));
						matchedKey = 1;
						break;
					}

					snprintf(testName, sizeof(testName), "keyblobMacKey_%02x", i);
					if (strcmp(key, testName) == 0) {
						uhx(keyblobMacKeys[i], value, sizeof(keyblobMacKeys[i]));
						matchedKey = 1;
						break;
					}

					snprintf(testName, sizeof(testName), "encryptedKeyblob_%02x", i);
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

					snprintf(testName, sizeof(testName), "masterKey_%02x", i);
					if (strcmp(key, testName) == 0) {
						uhx(masterKeys[i], value, sizeof(masterKeys[i]));
						matchedKey = 1;
						break;
					}

					snprintf(testName, sizeof(testName), "package1Key_%02x", i);
					if (strcmp(key, testName) == 0) {
						uhx(package1Keys[i], value, sizeof(package1Keys[i]));
						matchedKey = 1;
						break;
					}

					snprintf(testName, sizeof(testName), "package2Key_%02x", i);
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

					snprintf(testName, sizeof(testName), "keyAreaKeyApplication_%02x", i);
					if (strcmp(key, testName) == 0) {
						uhx(keyAreaKeys[i][0], value, sizeof(keyAreaKeys[i][0]));
						matchedKey = 1;
						break;
					}

					snprintf(testName, sizeof(testName), "keyAreaKeyOcean_%02x", i);
					if (strcmp(key, testName) == 0) {
						uhx(keyAreaKeys[i][1], value, sizeof(keyAreaKeys[i][1]));
						matchedKey = 1;
						break;
					}

					snprintf(testName, sizeof(testName), "keyAreaKeySystem_%02x", i);
					if (strcmp(key, testName) == 0) {
						uhx(keyAreaKeys[i][2], value, sizeof(keyAreaKeys[i][2]));
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
	return true;
}
#pragma once
#include "nx/primitives.h"
#include "nx/buffer.h"
#include "nx/integer.h"

void generateKek(integer<128>& dst, const integer<128> src, const integer<128> masterKey, const integer<128> kekSeed, const integer<128> keySeed);

class Keys
{
public:
	Keys();
	Keys(const char* fileName);
	bool open(const char* fileName);

	integer<128>& keyAreaKey(u8 masterKeyRev, u8 kaedIndex)
	{
		integer<128>& key = m_keyAreaKeys[masterKeyRev][kaedIndex];

		if (key == integer<128>(0))
		{
			generateKek(m_keyAreaKeys[masterKeyRev][kaedIndex], keyAreaKeyApplicationSource, masterKeys[masterKeyRev], aesKekGenerationSource, aesKeyGenerationSource);
		}

		return key;
	}

	integer<128> secureBootKey;                 /* Secure boot key for use in key derivation. NOTE: CONSOLE UNIQUE. */
	integer<128> tsecKey;                        /* TSEC key for use in key derivation. NOTE: CONSOLE UNIQUE. */
	integer<128> keyblobKeys[0x20];              /* Actual keys used to decrypt keyblobs. NOTE: CONSOLE UNIQUE.*/
	integer<128> keyblobMacKeys[0x20];          /* Keys used to validate keyblobs. NOTE: CONSOLE UNIQUE. */
	u8 encryptedKeyBlobs[0x20][0xB0];        /* Actual encrypted keyblobs (EKS). NOTE: CONSOLE UNIQUE. */
	u8 keyblobs[0x20][0x90];                  /* Actual decrypted keyblobs (EKS). */
	integer<128> keyblobKeySources[0x20];       /* Seeds for keyblob keys. */
	integer<128> keyblobMacKeySource;          /* Seed for keyblob MAC key derivation. */
	integer<128> masterKeySource;               /* Seed for master key derivation. */
	integer<128> masterKeys[0x20];               /* Firmware master keys. */
	integer<128> package1Keys[0x20];             /* Package1 keys. */
	integer<128> package2Keys[0x20];             /* Package2 keys. */
	integer<128> package2KeySource;             /* Seed for Package2 key. */
	integer<128> aesKekGenerationSource;       /* Seed for GenerateAesKek, usecase + generation 0. */
	integer<128> aesKeyGenerationSource;       /* Seed for GenerateAesKey. */
	integer<128> keyAreaKeyApplicationSource; /* Seed for kaek 0. */
	integer<128> keyAreaKeyOceanSource;       /* Seed for kaek 1. */
	integer<128> keyAreaKeySystemSource;      /* Seed for kaek 2. */
	integer<128> titlekekSource;                 /* Seed for titlekeks. */
	integer<128> headerKekSource;               /* Seed for header kek. */
	integer<128> sdCardKekSource;              /* Seed for SD card kek. */
	integer<256> sdCardKeySources[2];          /* Seed for SD card encryption keys. */
	integer<256> headerKeySource;               /* Seed for NCA header key. */
	integer<256> headerKey;                      /* NCA header key. */
	integer<128> titlekeks[0x20];                 /* Title key encryption keys. */
	integer<128> m_keyAreaKeys[0x20][3];          /* Key area encryption keys. */
	integer<256> sdCardKeys[2];
	integer<2018> ncaHdrFixedKeyModulus;      /* NCA header fixed key RSA pubk. */
	integer<2018> acidFixedKeyModulus;         /* ACID fixed key RSA pubk. */
	integer<2018> package2FixedKeyModulus;     /* Package2 Header RSA pubk. */
};

Keys& keys();

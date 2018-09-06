#pragma once
#include "nx/primitives.h"
#include "nx/buffer.h"

class Keys
{
public:
	Keys();
	Keys(const char* fileName);
	bool open(const char* fileName);

	u8 secureBootKey[0x10];                 /* Secure boot key for use in key derivation. NOTE: CONSOLE UNIQUE. */
	u8 tsecKey[0x10];                        /* TSEC key for use in key derivation. NOTE: CONSOLE UNIQUE. */
	u8 keyblobKeys[0x20][0x10];              /* Actual keys used to decrypt keyblobs. NOTE: CONSOLE UNIQUE.*/
	u8 keyblobMacKeys[0x20][0x10];          /* Keys used to validate keyblobs. NOTE: CONSOLE UNIQUE. */
	u8 encryptedKeyBlobs[0x20][0xB0];        /* Actual encrypted keyblobs (EKS). NOTE: CONSOLE UNIQUE. */
	u8 keyblobs[0x20][0x90];                  /* Actual decrypted keyblobs (EKS). */
	u8 keyblobKeySources[0x20][0x10];       /* Seeds for keyblob keys. */
	u8 keyblobMacKeySource[0x10];          /* Seed for keyblob MAC key derivation. */
	u8 masterKeySource[0x10];               /* Seed for master key derivation. */
	u8 masterKeys[0x20][0x10];               /* Firmware master keys. */
	u8 package1Keys[0x20][0x10];             /* Package1 keys. */
	u8 package2Keys[0x20][0x10];             /* Package2 keys. */
	u8 package2KeySource[0x10];             /* Seed for Package2 key. */
	u8 aesKekGenerationSource[0x10];       /* Seed for GenerateAesKek, usecase + generation 0. */
	u8 aesKeyGenerationSource[0x10];       /* Seed for GenerateAesKey. */
	u8 keyAreaKeyApplicationSource[0x10]; /* Seed for kaek 0. */
	u8 keyAreaKeyOceanSource[0x10];       /* Seed for kaek 1. */
	u8 keyAreaKeySystemSource[0x10];      /* Seed for kaek 2. */
	u8 titlekekSource[0x10];                 /* Seed for titlekeks. */
	u8 headerKekSource[0x10];               /* Seed for header kek. */
	u8 sdCardKekSource[0x10];              /* Seed for SD card kek. */
	u8 sdCardKeySources[2][0x20];          /* Seed for SD card encryption keys. */
	u8 headerKeySource[0x20];               /* Seed for NCA header key. */
	u8 headerKey[0x20];                      /* NCA header key. */
	u8 titlekeks[0x20][0x10];                 /* Title key encryption keys. */
	u8 keyAreaKeys[0x20][3][0x10];          /* Key area encryption keys. */
	u8 sdCardKeys[2][0x20];
	u8 ncaHdrFixedKeyModulus[0x100];      /* NCA header fixed key RSA pubk. */
	u8 acidFixedKeyModulus[0x100];         /* ACID fixed key RSA pubk. */
	u8 package2FixedKeyModulus[0x100];     /* Package2 Header RSA pubk. */
};

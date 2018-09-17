#pragma once
#include "nx/file.h"

class TicketData;

class TicketHeader
{
public:
	static const u32 RSA_4096_SHA1 = 0x010000;
	static const u32 RSA_2048_SHA1 = 0x010001;
	static const u32 ECDSA_SHA1 = 0x010002;
	static const u32 RSA_4096_SHA256 = 0x010003;
	static const u32 RSA_2048_SHA256 = 0x010004;
	static const u32 ECDSA_SHA256 = 0x010005;

	u32& signatureType() { return m_signatureType; }

	u8 padding()
	{
		switch (signatureType())
		{
		case RSA_4096_SHA1:
			return 0x3C;
		case RSA_2048_SHA1:
			return 0x3C;
		case ECDSA_SHA1:
			return 0x40;
		case RSA_4096_SHA256:
			return 0x3C;
		case RSA_2048_SHA256:
			return 0x3C;
		case ECDSA_SHA256:
			return 0x40;
		}
		return 0;
	}

	u8* signature()
	{
		return (u8*)m_signature;
	}

	u16 signatureSize()
	{
		switch (signatureType())
		{
		case RSA_4096_SHA1:
			return 0x200;
		case RSA_2048_SHA1:
			return 0x100;
		case ECDSA_SHA1:
			return 0x3C;
		case RSA_4096_SHA256:
			return 0x200;
		case RSA_2048_SHA256:
			return 0x100;
		case ECDSA_SHA256:
			return 0x3C;
		}
		return 0;
	}

	TicketData* ticketData()
	{
		return reinterpret_cast<TicketData*>(signature() + signatureSize() + padding());
	}

private:
	u32 m_signatureType;
	u8 m_signature[0x200];
};

class TicketData
{
public:
	integer<512> issuer;
	integer<2048> titleKeyBlock;
	u8 unknown1;
	u8 titleKeyType;
	u8 unknown2[3];
	u8 masterKeyRevision;
	u8 unknown3[0xA];
	u64 ticketId;
	u64 deviceId;
	RightsId rightsId;
	u32 accountId;
	u8 unknown4[0xC];
	u8 unknown5[0x140];
};

class Ticket : public File
{
public:
	Ticket();
	bool init() override;
private:
};

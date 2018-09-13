#pragma once
#include <stdint.h>
/*
typedef __int8            int8_t;
typedef __int16           int16_t;
typedef __int32           int32_t;
typedef __int64           int64_t;
typedef unsigned __int8   uint8_t;
typedef unsigned __int16  uint16_t;
typedef unsigned __int32  uint32_t;
typedef unsigned __int64  uint64_t;
*/

#ifdef _MSC_VER
#define PACKED
#endif

#define _MAX(x, y) (((x) > (y)) ? (x) : (y))
#define _MIN(x, y) (((x) < (y)) ? (x) : (y))

template <typename T> const char* mapTypeExtension();

#ifdef _MSC_VER
typedef uint8_t u8;       ///<   8-bit unsigned integer.
typedef uint16_t u16;     ///<  16-bit unsigned integer.
typedef uint32_t u32;     ///<  32-bit unsigned integer.
typedef uint64_t u64;     ///<  64-bit unsigned integer.
						  //typedef __uint128_t u128; ///< 128-bit unsigned integer.

typedef int8_t s8;       ///<   8-bit signed integer.
typedef int16_t s16;     ///<  16-bit signed integer.
typedef int32_t s32;     ///<  32-bit signed integer.
typedef int64_t s64;     ///<  64-bit signed integer.
						 //typedef __int128_t s128; ///< 128-bit unsigned integer.

typedef volatile u8 vu8;     ///<   8-bit volatile unsigned integer.
typedef volatile u16 vu16;   ///<  16-bit volatile unsigned integer.
typedef volatile u32 vu32;   ///<  32-bit volatile unsigned integer.
typedef volatile u64 vu64;   ///<  64-bit volatile unsigned integer.
							 //typedef volatile u128 vu128; ///< 128-bit volatile unsigned integer.

typedef volatile s8 vs8;     ///<   8-bit volatile signed integer.
typedef volatile s16 vs16;   ///<  16-bit volatile signed integer.
typedef volatile s32 vs32;   ///<  32-bit volatile signed integer.
typedef volatile s64 vs64;   ///<  64-bit volatile signed integer.
							 //typedef volatile s128 vs128; ///< 128-bit volatile signed integer.

#define Handle u32
#define Result u32
#else
#include <switch/types.h>
#endif


#define NcaId integer<128>

class TitleId
{
public:
	TitleId() : id(0)
	{
	}

	TitleId(const u64 src) : id(src)
	{
	}

	operator u64() const
	{
		return id;
	}

	TitleId baseId() const
	{
		return id & 0xFFFFFFFFFFFFE000;
	}

	TitleId updateId() const
	{
		if (isDlc())
		{
			return 0;
		}
		return (id & 0xFFFFFFFFFFFFF000) + 0x800;
	}

	bool isDlc() const
	{
		return (id & 0xFFFFFFFFFFFFEFFF) != id;
	}

	u64 id;
} PACKED;

class RightsId
{
public:
	RightsId() : m_titleId(0), m_masterKeyRev(0)
	{
	}

	const TitleId& titleId() const { return m_titleId; }
	TitleId& titleId() { return m_titleId; }

	u64 masterKeyRev() const { return m_masterKeyRev; }
	void setMasterKeyRev(u64 val) { m_masterKeyRev = val; }

private:
	TitleId m_titleId;
	u64 m_masterKeyRev;
}  PACKED;
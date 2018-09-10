#pragma once

#ifdef _MSC_VER
#include <winsock2.h>
#include <windows.h>

#define svcSleepThread(s) Sleep(s / 1000000)

bool appletMainLoop();

/// The maximum value of a u64.
#define U64_MAX	UINT64_MAX

#ifndef SSIZE_MAX
#ifdef SIZE_MAX
#define SSIZE_MAX ((SIZE_MAX) >> 1)
#endif
#endif

#include "nx/primitives.h"

typedef void(*ThreadFunc)(void *); ///< Thread entrypoint function.
typedef void(*VoidFn)(void);       ///< Function without arguments nor return value.

								   /// Creates a bitmask from a bit number.
#define BIT(n) (1U<<(n))

								   /// Aligns a struct (and other types?) to m, making sure that the size of the struct is a multiple of m.
#define ALIGN(m)   __attribute__((aligned(m)))
								   /// Packs a struct (and other types?) so it won't include padding bytes.
#define PACKED

#define NORETURN   __attribute__((noreturn))
#define IGNORE_ARG(x) (void)(x)

#ifndef LIBNX_NO_DEPRECATION
								   /// Flags a function as deprecated.
#define DEPRECATED __attribute__ ((deprecated))
#else
								   /// Flags a function as deprecated.
#define DEPRECATED
#endif

#define INVALID_HANDLE ((Handle) 0)

enum FsStorageId
{
	FsStorageId_None = 0,
	FsStorageId_Host = 1,
	FsStorageId_GameCard = 2,
	FsStorageId_NandSystem = 3,
	FsStorageId_NandUser = 4,
	FsStorageId_SdCard = 5
};

struct NcmMetaRecord
{
	u64 titleId;
	u32 version;
	u8  type;
	u8  flags;
	u8  padding[2];
};

typedef u32 NcmContentStorage;
typedef u32 NcmContentMetaDatabase;
typedef u32 Service;

#else
#include <switch.h>
#include <switch/arm/atomics.h>
#define closesocket close
#endif
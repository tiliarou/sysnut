#pragma once

#include "nut.h"

class Lock
{
public:
	Lock() : readMutex(0), writeMutex(0)
	{
	}

	bool acquireReadLock(u32 timeout = 5000)
	{
		u64 i = 0;
		while (writeMutex)
		{
			if (i++ > timeout)
			{
				return false;
			}
#ifdef __SWITCH__
			svcSleepThread(_1MS);
#elif _MSC_VER
			Sleep(_1MS);
#endif
		}
#ifdef __SWITCH__
		atomicIncrement64(&readMutex);
#else
		readMutex++;
#endif
		return true;
	}

	bool releaseReadLock()
	{
		if (readMutex)
		{
#ifdef __SWITCH__
			atomicDecrement64(&readMutex);
#else
			readMutex--;
#endif
			return true;
		}
		return false;
	}

	bool acquireWriteLock(u32 timeout = 5000)
	{
		u64 i = 0;
		while (readMutex || writeMutex)
		{
			if (++i > timeout)
			{
				return false;
			}
#ifdef __SWITCH__
			svcSleepThread(_1MS);
#endif
		}
#ifdef __SWITCH__
		atomicIncrement64(&writeMutex);
#else
		writeMutex++;
#endif
		return true;
	}

	bool releaseWriteLock()
	{
		if (writeMutex)
		{
#ifdef __SWITCH__
			atomicDecrement64(&writeMutex);
#else
			writeMutex--;
#endif
			return true;
		}
		return false;
	}

private:
	u64 readMutex;
	u64 writeMutex;
};
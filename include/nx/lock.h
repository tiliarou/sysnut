#pragma once

#include "nut.h"

class Lock
{
public:
	Lock() : readMutex(0), writeMutex(0)
	{
	}

	bool acquireReadLock()
	{
		u64 i = 0;
		while (writeMutex)
		{
#ifdef __SWITCH__
			svcSleepThread(_1MS);
#elif _MSC_VER
			Sleep(_1MS);
#endif
			if (i++ > 5000)
			{
				return false;
			}
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

	bool acquireWriteLock()
	{
		u64 i = 0;
		while (readMutex || writeMutex)
		{
#ifdef __SWITCH__
			svcSleepThread(_1MS);
#endif
			if (i++ > 5000)
			{
				return false;
			}
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
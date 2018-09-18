#pragma once

#include "nx/buffer.h"
#include "nx/lock.h"

template<class T, u64 SZ>
class ChineseSdBuffer
{
public:
	ChineseSdBuffer()
	{
		circularStartPosition() = 0;
		circularEndPosition() = 0;
		//circularCurrentPosition() = 0;
	}

	u64 size()
	{
		return circularEndPosition() - circularStartPosition();
	}

	bool isFull()
	{
		return size() >= SZ;
	}

	bool canWrite()
	{
		u64 c = 0;

		while (isFull())
		{
			if (c++ > 500)
			{
				return false;
			}
			nxSleep(10);
		}
		return true;
	}

	bool canRead()
	{
		u64 c = 0;

		while (!size())
		{
			if (c++ > 500)
			{
				return false;
			}
			nxSleep(10);
		}
		return true;
	}

	T* push()
	{
		if (!canWrite())
		{
			return NULL;
		}

		if (isFull())
		{
			circularStartPosition()++;
			circularEndPosition()++;
		}
		else
		{
			circularEndPosition()++;
		}
		return last();
	}

	T* push(T n)
	{
		if (!canWrite())
		{
			return NULL;
		}

		if (isFull())
		{
			*first() = n;
			circularStartPosition()++;
			circularEndPosition()++;
		}
		else
		{
			circularEndPosition()++;
			*last() = n;
		}
		return last();
	}

	bool shift()
	{
		if (!canRead())
		{
			return false;
		}

		circularStartPosition()++;
		return true;
	}

	T* last()
	{
		if (!canRead())
		{
			return NULL;
		}

		auto i = (circularEndPosition() - 1) % SZ;
		return &m_buffer[i];
	}

	T* peek()
	{
		auto i = (circularEndPosition()) % SZ;
		return &m_buffer[i];
	}

	T* first()
	{
		if (!canRead())
		{
			return NULL;
		}
		return &m_buffer[circularStartPosition() % SZ];
	}

	/*const T& operator[](u64 i) const
	{
		return m_buffer[(i + circularStartPosition()) % SZ];
	}

	T& operator[](u64 i)
	{
		return m_buffer[(i + circularStartPosition()) % SZ];
	}*/

	u64& circularStartPosition() { return m_circularStartPosition; }
	u64& circularEndPosition() { return m_circularEndPosition; }
private:
	Lock lock;
	u64 m_circularStartPosition;
	u64 m_circularEndPosition;

	T m_buffer[SZ];
};
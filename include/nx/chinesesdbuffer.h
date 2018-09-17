#pragma once

#include "nx/buffer.h"

template<class T, u64 SZ, bool OVERWRITE = false>
class ChineseSdBuffer : public Buffer<T>
{
public:
	ChineseSdBuffer() : Buffer<T>()
	{
		Buffer<T>::resize(SZ);
		Buffer<T>::resize(0);
		circularStartPosition() = 0;
		circularEndPosition() = 0;
		//circularCurrentPosition() = 0;
	}

	u64 range()
	{
		return circularEndPosition() - circularStartPosition();
	}

	u64 size2()
	{
		return circularEndPosition() - circularStartPosition();
	}

	bool isFull()
	{
		return range() >= SZ;
	}

	bool push(T n)
	{
		if (m_size < SZ)
		{
			Buffer<T>::push(n);
			circularEndPosition()++;
			return true;
		}

		if (!OVERWRITE && isFull())
		{
			return false;
		}

		//Buffer<T>::m_buffer[(circularEndPosition()-1) % SZ] = n;
		if (isFull())
		{
			first() = n;
			circularStartPosition()++;
			circularEndPosition()++;
		}
		else
		{
			last() = n;
			//circularStartPosition()++;
			circularEndPosition()++;
		}
		return true;
	}

	bool shift()
	{
		if (!m_size)
		{
			return false;
		}

		circularStartPosition()++;
	}

	T& last()
	{
		if (!m_size)
		{
			return (*this)[0];
		}
		return (*this)[(circularEndPosition() - 1) % SZ];
	}

	T& first()
	{
		if (!m_size)
		{
			return (*this)[0];
		}
		return (*this)[circularStartPosition() % SZ];
	}

	const T& operator[](u64 i) const
	{
		return Buffer<T>::m_buffer[(i + circularStartPosition()) % SZ];
	}

	T& operator[](u64 i)
	{
		return Buffer<T>::m_buffer[(i + circularStartPosition()) % SZ];
	}

	//u64& circularCurrentPosition() { return m_circularCurrentPosition; }
	u64& circularStartPosition() { return m_circularStartPosition; }
	u64& circularEndPosition() { return m_circularEndPosition; }
private:
	//u64 m_circularCurrentPosition;
	u64 m_circularStartPosition;
	u64 m_circularEndPosition;
};
#pragma once

#include "nx/buffer.h"

template<class T, u64 SZ>
class CircularBuffer
{
public:
	CircularBuffer()
	{
		circularPosition() = 0;
		size() = 0;
	}

	T& push(T n)
	{
		if (size() < SZ)
		{
			m_buffer[size()] = n;
			return m_buffer[size()++];
		}

		m_buffer[circularPosition() % SZ] = n;

		return m_buffer[circularPosition()++ % SZ];
	}

	T& push()
	{
		if (size() < SZ)
		{
			return m_buffer[size()++];
		}

		return m_buffer[circularPosition()++ % SZ];
	}

	bool isFull()
	{
		return size() >= SZ;
	}

	const T& operator[](u32 i) const
	{
		return m_buffer[(i + m_circularPosition) % SZ];
	}

	T& operator[](u32 i)
	{
		return m_buffer[(i + m_circularPosition) % SZ];
	}

	u64& size() { return m_size; }

	T& last()
	{
		if (!size())
		{
			return m_buffer[0];
		}

		if (isFull())
		{
			return m_buffer[(circularPosition() - 1) % SZ];
		}
		else
		{
			return m_buffer[size() - 1];
		}
	}

	T& first()
	{
		return m_buffer[circularPosition() % SZ];
	}

	u32& circularPosition() { return m_circularPosition; }
private:
	u64 m_size;
	u32 m_circularPosition;
	T m_buffer[SZ];
};
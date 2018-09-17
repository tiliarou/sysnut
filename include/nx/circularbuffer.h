#pragma once

#include "nx/buffer.h"

template<class T, u64 SZ>
class CircularBuffer : public Buffer<T>
{
public:
	CircularBuffer() : Buffer<T>()
	{
		Buffer<T>::resize(SZ);
		Buffer<T>::resize(0);
		circularPosition() = 0;
	}

	T& push(T n)
	{
		if (Buffer<T>::size() < SZ)
		{
			return Buffer<T>::push(n);
		}
		Buffer<T>::m_buffer[circularPosition()] = n;
		circularPosition() = (circularPosition() + 1) % SZ;
		return Buffer<T>::m_buffer[circularPosition()];
	}

	const T& operator[](u32 i) const
	{
		return Buffer<T>::m_buffer[(i + m_circularPosition) % SZ];
	}

	T& operator[](u32 i)
	{
		return Buffer<T>::m_buffer[(i + m_circularPosition) % SZ];
	}

	u32& circularPosition() { return m_circularPosition; }
private:
	u32 m_circularPosition;
};
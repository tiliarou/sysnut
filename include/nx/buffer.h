#pragma once
#include "nut.h"

#define BUFFER_ALIGN 0xFF

class Buffer
{
public:
	Buffer();
	~Buffer();

	bool resize(u64 newSize);

	const void* buffer() const { return m_buffer; }
	void*& buffer() { return m_buffer; }
	u64& size() { return m_size;  }
	u64& bufferSize() { return m_bufferSize; }

	bool set(const void* src, u64 sz);
	bool close();

	char* c_str() const;

private:
	Buffer(Buffer const&);
	Buffer& operator=(Buffer const&);

	void* m_buffer = NULL;
	u64 m_size = 0;
	u64 m_bufferSize = 0;
};
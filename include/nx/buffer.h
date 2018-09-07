#pragma once
#include "nut.h"

#define BUFFER_ALIGN 0xFF

class Buffer
{
public:
	Buffer();
	~Buffer();

	Buffer(const Buffer& src);
	Buffer& operator=(const Buffer& src);

	bool slice(Buffer& out, s64 start, s64 end) const;

	bool resize(u64 newSize);

	const void* buffer() const { return m_buffer; }
	void*& buffer() { return m_buffer; }

	const u64& size() const { return m_size; }
	u64& size() { return m_size;  }

	const u64& bufferSize() const { return m_bufferSize; }
	u64& bufferSize() { return m_bufferSize; }

	bool set(const void* src, u64 sz);
	bool close();

	char* c_str() const;

	void dump(int sz = 0, int offset = 0) const;

private:

	void* m_buffer = NULL;
	u64 m_size = 0;
	u64 m_bufferSize = 0;
};
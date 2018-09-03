#pragma once
#include <switch/types.h>

class Buffer
{
public:
	Buffer();
	~Buffer();

	bool resize(u64 newSize);

	const void* buffer() const { return m_buffer; }
	void*& buffer() { return m_buffer; }
	u64& size() { return m_size;  }

	char* c_str() const;

private:
	void* m_buffer = NULL;
	u64 m_size = 0;
};
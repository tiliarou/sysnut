#pragma once
#include "nx/buffer.h"

class string : public Buffer<u8>
{
public:
	string();
	string(const char* s);
	string(const string& src);
	string& operator=(const string& src);

	~string();
	bool set(const void* src, u64 sz=0);
	operator char*() const { return c_str(); }

	const u64 length() const { return m_size ? m_size - 1 : 0; }

	bool startsWith(string& s);
	bool endsWith(string& s);
private:
};
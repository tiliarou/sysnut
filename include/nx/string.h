#pragma once
#include "nx/buffer.h"
#include "nx/primitives.h"

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

	const u64 length() const { return _MIN(m_size ? m_size - 1 : 0, strlen((const char*)buffer())); }

	bool startsWith(string& s);
	bool endsWith(string& s);
private:
};

static char itohex(u8 nibble, bool cap = false)
{
	if (nibble < 0xA)
	{
		return '0' + nibble;
	}
	else if (nibble < 0x10)
	{
		return (cap?'A':'a') + (nibble - 0xA);
	}
	return NULL;
}

template<class T>
string hx(T& data)
{
	string result;
	result.resize(sizeof(data) * 2);
	result.resize(0);

	u8* p = reinterpret_cast<u8*>(&data);
	for (u32 i = 0; i < _MIN(sizeof(data), 0x100); i++)
	{
		result.push(itohex(p[i] >> 4));
		result.push(itohex(p[i] & 0xF));
	}
	result.push(NULL);
	return result;
}
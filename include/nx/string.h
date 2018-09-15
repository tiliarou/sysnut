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
	operator const char*() const { return c_str(); }
	string operator+(const char* s);
	string& operator+=(const char* s);

	string& operator+=(const string s);

	bool operator==(const string& s);
	bool operator==(const char* s);

	string& trim();

	sptr<Array<string>> split(char c);

	u64 length() const { return _MIN(m_size ? m_size - 1 : 0, strlen((const char*)buffer())); }

	bool startsWith(string s);
	bool endsWith(string s);
private:
};

int ishex(char c);
char hextoi(char c);
char itohex(u8 nibble, bool cap = false);

template<class T>
string hx(const T& data)
{
	string result;
	result.resize(sizeof(data) * 2);
	result.resize(0);

	const u8* p = reinterpret_cast<const u8*>(&data);
	for (u32 i = 0; i < _MIN(sizeof(data), 0x100); i++)
	{
		result.push(itohex(p[i] >> 4));
		result.push(itohex(p[i] & 0xF));
	}
	result.push('\0');
	return result;
}

Buffer<u8> uhx(string hex);
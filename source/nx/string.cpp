#include "nx/string.h"

string::string() : Buffer()
{
}

string::string(const char* s)
{
	set(s, 0);
}

string::string(const string& src)
{
	*this = src;
}

bool string::operator==(const string& s) const
{
	if (s.length() != length())
	{
		return false;
	}
	return memcmp(buffer(), s.buffer(), length()) == 0;
}

bool string::operator==(const char* s) const
{
	u64 len = (u64)strlen(s);

	if (len != length())
	{
		return false;
	}

	return memcmp(buffer(), s, size()) == 0;
}

string& string::operator=(const string& src)
{
	src.slice(*this, 0, src.size());
	//push(NULL);
	return *this;
}

string& string::operator+=(const char* s)
{
	size_t len = strlen(s);

	resize(length() + len + 1);

	memcpy(this->buffer() + length(), s, len);

	if (!size() || last() != NULL)
	{
		push(NULL);
	}
	return *this;
}

string& string::operator+=(const string s)
{
	resize(length() + s.length() + 1);

	memcpy(this->buffer() + length(), s, s.length());

	if (!size() || last() != NULL)
	{
		push(NULL);
	}
	return *this;
}

string string::operator+(const char* s) const
{
	string result = *this;
	result += s;
	return result;
	/*size_t len = strlen(s);

	result.resize(length() + len + 1);

	memcpy(result.buffer() + length(), s, len);

	if (!result.size() || result.last() != NULL)
	{
		push(NULL);
	}
	return result;*/
}

bool string::set(const void* src, u64 sz)
{
	if (sz == 0)
	{
		sz = strlen((const char*)src);
	}

	if(!Buffer::set(src, sz))
	{
		return false;
	}

	if (!size() || last() != NULL)
	{
		push(NULL);
	}

	return true;
}

string::~string()
{
}

bool string::startsWith(string s)
{
	if (s.size() > size())
	{
		return false;
	}

	for (int i = 0; i < s.length(); i++)
	{
		if (*c_str(i) != *s.c_str(i))
		{
			return false;
		}
	}

	return true;
}

bool string::endsWith(string s)
{
	if (s.size() > size())
	{
		return false;
	}

	for (u64 i = 0, j=length() - s.length(); i < s.length(); i++, j++)
	{
		if (*c_str(j) != *s.c_str(i))
		{
			return false;
		}
	}

	return true;
}

int ishex(char c)
{
	if ('a' <= c && c <= 'f') return 1;
	if ('A' <= c && c <= 'F') return 1;
	if ('0' <= c && c <= '9') return 1;
	return 0;
}

char hextoi(char c)
{
	if ('a' <= c && c <= 'f') return c - 'a' + 0xA;
	if ('A' <= c && c <= 'F') return c - 'A' + 0xA;
	if ('0' <= c && c <= '9') return c - '0';
	return 0;
}

char itohex(u8 nibble, bool cap)
{
	if (nibble < 0xA)
	{
		return '0' + nibble;
	}
	else if (nibble < 0x10)
	{
		return (cap ? 'A' : 'a') + (nibble - 0xA);
	}
	return '\0';
}

Buffer<u8> uhx(string hex)
{
	Buffer<u8> key;
	u32 len = 0;

	for (u32 i = 0; i < hex.size(); i++)
	{
		if (!ishex(hex[i]))
		{
			break;
		}
		len++;
	}

	if (!len)
	{
		return key;
	}

	if (len % 2)
	{
		error("Hex string (%s) must be an even number of characters!\n", hex, len);
		return key;
	}
	key.resize(len >> 1);
	memset(key.buffer(), 0, (size_t)key.size());
	u8* ptr = key.buffer();

	for (unsigned int i = 0; i < len; i++)
	{
		char val = hextoi(hex[i]);

		if ((i & 1) == 0)
		{
			val <<= 4;
		}
		ptr[i >> 1] |= val;
	}

	return key;
}

sptr<Array<string>> string::split(char c)
{
	sptr<Array<string>> result(new Array<string>());

	u32 start = 0;
	for (u32 i = 0; i < length(); i++)
	{
		if ((*this)[i] == c)
		{
			string buffer;
			slice(buffer, start, i);
			buffer.push('\0');
			result->push(buffer);
			start = i + 1;
		}
	}

	string buffer;
	slice(buffer, start, length());
	buffer.push('\0');
	result->push(buffer);

	return result;
}

string& string::trim()
{
	u32 i = size();

	while (i > 0)
	{
		i--;

		char c = (*this)[i];
		if (c == '\r' || c == '\n' || c == '\0' || c == ' ' || c == '\t')
		{
			pop();
		}
		else
		{
			break;
		}
	}
	push('\0');
	return *this;
}
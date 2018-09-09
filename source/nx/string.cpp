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

string& string::operator=(const string& src)
{
	src.slice(*this, 0, src.size());
	//push(NULL);
	return *this;
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

	push(NULL);

	return true;
}

string::~string()
{
}

bool string::startsWith(string& s)
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

bool string::endsWith(string& s)
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

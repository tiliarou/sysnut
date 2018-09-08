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
	return *this;
}

bool string::set(const void* src, u64 sz)
{
	if (sz == 0)
	{
		sz = strlen((const char*)src);
	}

	if(!Buffer::set(src, sz + 1))
	{
		return false;
	}

	this->c_str()[sz] = NULL;

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

	for (int i = 0; i < s.size(); i++)
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

	for (u64 i = 0, j=size() - s.size(); i < s.size(); i++, j++)
	{
		if (*c_str(j) != *s.c_str(i))
		{
			return false;
		}
	}

	return true;
}

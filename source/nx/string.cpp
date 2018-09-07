#include "nx/string.h"

string::string() : Buffer()
{
}

string::string(const char* s)
{
	u64 len = strlen(s)+1;
	resize(len);
	set(s, len);
	c_str()[len - 1] = NULL;
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

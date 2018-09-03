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

string::~string()
{
}

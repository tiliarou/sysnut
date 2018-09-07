#pragma once
#include "nx/buffer.h"

class string : public Buffer
{
public:
	string();
	string(const char* s);
	string(const string& src);
	string& operator=(const string& src);

	~string();
	bool set(const void* src, u64 sz=0);
	operator char*() const { return c_str(); }
private:
};
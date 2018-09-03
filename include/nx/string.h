#pragma once
#include "nx/buffer.h"

class string : public Buffer
{
public:
	string();
	string(const char* s);
	~string();
	operator char*() const { return c_str(); }
private:
};
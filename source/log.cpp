#include <stdio.h>
#include <stdarg.h>

FILE* f = NULL;


void print(const char * format, ...)
{
	if (!f)
	{
		f = fopen("/nutd.log", "a");
	}

	va_list args;
	va_start(args, format);
	vfprintf(f, format, args);
	va_end(args);
	fflush(f);
}
#include <stdio.h>
#include <stdarg.h>
#include "socket.h"

FILE* f = NULL;

extern Socket* currentSocket;


void print(const char * format, ...)
{
	if (!f)
	{
		f = fopen("/nutd.log", "a");
	}

	va_list args;
	va_start(args, format);
	vfprintf(f, format, args);
	if (currentSocket)
	{
		vdprintf((int)*currentSocket, format, args);
	}
	va_end(args);
	fflush(f);
}
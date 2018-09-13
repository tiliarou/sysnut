#include <stdio.h>
#include <stdarg.h>
#include "socket.h"

FILE* f = NULL;

extern Socket* currentSocket;

FILE* file()
{
	if (!f)
	{
#ifdef _MSC_VER
		f = stdout;
#else
		f = stdout;
		//f = fopen("/nutd.log", "a");
#endif
	}

	return f;
}


void print(const char * format, ...)
{
	va_list args;
	va_start(args, format);
	vfprintf(file(), format, args);
#ifndef _MSC_VER
	if (currentSocket)
	{
		vdprintf((int)*currentSocket, format, args);
	}
#endif
	va_end(args);
	fflush(f);
}

void fatal(const char * format, ...)
{
	va_list args;
	va_start(args, format);
	vfprintf(file(), format, args);
#ifndef _MSC_VER
	if (currentSocket)
	{
		vdprintf((int)*currentSocket, format, args);
	}
#endif
	va_end(args);
	fflush(f);
}

void error(const char * format, ...)
{
	va_list args;
	va_start(args, format);
	vfprintf(file(), format, args);
#ifndef _MSC_VER
	if (currentSocket)
	{
		vdprintf((int)*currentSocket, format, args);
	}
#endif
	va_end(args);
	fflush(f);
}

void warning(const char * format, ...)
{
	va_list args;
	va_start(args, format);
	vfprintf(file(), format, args);
#ifndef _MSC_VER
	if (currentSocket)
	{
		vdprintf((int)*currentSocket, format, args);
	}
#endif
	va_end(args);
	fflush(f);
}

void debug(const char * format, ...)
{
	va_list args;
	va_start(args, format);
	vfprintf(file(), format, args);
#ifndef _MSC_VER
	if (currentSocket)
	{
		vdprintf((int)*currentSocket, format, args);
	}
#endif
	va_end(args);
	fflush(f);
}
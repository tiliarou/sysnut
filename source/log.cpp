#include <stdio.h>
#include <stdarg.h>
#include "nx/string.h"
#include "socket.h"
#include "log.h"

Array<PrintHook>* g_hooks = NULL;

Array<PrintHook>* hooks()
{
	if (!g_hooks)
	{
		g_hooks = new Array<PrintHook>();
	}
	return g_hooks;
}

void registerPrintHook(PrintHook func)
{
	hooks()->push(func);
}


FILE* f = NULL;

CircularBuffer<string, 0x100>* g_printLog = NULL;
CircularBuffer<string, 0x100>& printLog()
{
	if (!g_printLog)
	{
		g_printLog = new CircularBuffer<string, 0x100>();
	}
	return *g_printLog;
}

char printBuffer[0x1000];

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
	vsprintf(printBuffer, format, args);
	printf("%s", printBuffer);

	if (printBuffer[0] == '\r')
	{
		printLog().push(printBuffer+1);
	}
	else
	{
		printLog().push(printBuffer);
	}
	/*
	size_t len = strlen(printBuffer);

	if (!printLog().size())
	{
		printLog().push(string(""));
	}

	printLog().last() += printBuffer;*/

	va_end(args);
	fflush(f);

	for (auto& f : *hooks())
	{
		f();
	}

	/*if (printBuffer[len - 1] == '\n')
	{
		printLog().push(string(""));
	}*/
}

/*

void fatal(const char * format, ...)
{
	va_list args;
	va_start(args, format);
	vsprintf(printBuffer, format, args);
	printf("%s", printBuffer);
	printLog().push(printBuffer);

	va_end(args);
	fflush(f);

	for (auto& f : g_hooks)
	{
		f();
	}
}

void error(const char * format, ...)
{
	va_list args;
	va_start(args, format);
	vsprintf(printBuffer, format, args);
	printf("%s", printBuffer);
	printLog().push(printBuffer);

	va_end(args);
	fflush(f);

	for (auto& f : g_hooks)
	{
		f();
	}
}

void warning(const char * format, ...)
{
	va_list args;
	va_start(args, format);
	vsprintf(printBuffer, format, args);
	printf("%s", printBuffer);
	printLog().push(printBuffer);

	va_end(args);
	fflush(f);

	for (auto& f : g_hooks)
	{
		f();
	}
}

void debug(const char * format, ...)
{
	va_list args;
	va_start(args, format);
	vsprintf(printBuffer, format, args);
	printf("%s", printBuffer);
	printLog().push(printBuffer);

	va_end(args);
	fflush(f);

	for (auto& f : g_hooks)
	{
		f();
	}
}
*/
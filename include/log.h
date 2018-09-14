#pragma once

typedef void(*PrintHook)();

void print(const char * format, ...);
void fatal(const char * format, ...);
void error(const char * format, ...);
void warning(const char * format, ...);
void debug(const char * format, ...);

void registerPrintHook(PrintHook func);


#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

#include "alice/core.h"

char* alice_copy_string(const char* string) {
	assert(string);

	const u32 len = strlen(string);

	char* result = malloc(len + 1);

	strcpy(result, string);
	result[len] = '\0';

	return result;
}

u32 alice_hash_string(const char* string) {
	assert(string);

	const u32 len = strlen(string);

	u32 hash = 2166136261u;
	for (u32 i = 0; i < len; i++) {
		hash ^= string[i];
		hash *= 16777619;
	}

	return hash;
}

#if defined(ALICE_PLATFORM_LINUX)

#define CONSOLE_COLOR_RESET "\033[0m"
#define CONSOLE_COLOR_GREEN "\033[1;32m"
#define CONSOLE_COLOR_RED "\033[1;31m"
#define CONSOLE_COLOR_PURPLE "\033[1;35m"
#define CONSOLE_COLOR_CYAN "\033[0;36m"

void alice_log(const char* fmt, ...) {
	char final_format[1024];
	snprintf(final_format, 1024, "%sinfo%s: %s\n", CONSOLE_COLOR_GREEN,
		CONSOLE_COLOR_RESET, fmt);

	va_list argptr;
	va_start(argptr, fmt);
	vprintf(final_format, argptr);
	va_end(argptr);
}

void alice_log_warning(const char* fmt, ...) {
	char final_format[1024];
	snprintf(final_format, 1024, "%swarning%s: %s\n", CONSOLE_COLOR_PURPLE,
		CONSOLE_COLOR_RESET, fmt);

	va_list argptr;
	va_start(argptr, fmt);
	vprintf(final_format, argptr);
	va_end(argptr);
}

void alice_log_error(const char* fmt, ...) {
	char final_format[1024];
	snprintf(final_format, 1024, "%serror%s: %s\n", CONSOLE_COLOR_RED,
		CONSOLE_COLOR_RESET, fmt);

	va_list argptr;
	va_start(argptr, fmt);
	vprintf(final_format, argptr);
	va_end(argptr);
}

#elif defined(ALICE_PLATFORM_WINDOWS)
#include <windows.h>

void alice_log(const char* fmt, ...) {
	HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);

	SetConsoleTextAttribute(console, 2);
	printf("info: ");
	SetConsoleTextAttribute(console, 7);
	
	va_list argptr;
	va_start(argptr, fmt);
	vprintf(fmt, argptr);
	va_end(argptr);

	printf("\n");
}

void alice_log_error(const char* fmt, ...) {
	HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);

	SetConsoleTextAttribute(console, 4);
	printf("error: ");
	SetConsoleTextAttribute(console, 7);
	
	va_list argptr;
	va_start(argptr, fmt);
	vprintf(fmt, argptr);
	va_end(argptr);

	printf("\n");
}

void alice_log_warning(const char* fmt, ...) {
	HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);

	SetConsoleTextAttribute(console, 6);
	printf("warning: ");
	SetConsoleTextAttribute(console, 7);
	
	va_list argptr;
	va_start(argptr, fmt);
	vprintf(fmt, argptr);
	va_end(argptr);

	printf("\n");
}
#endif

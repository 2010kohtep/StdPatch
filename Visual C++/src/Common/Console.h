#pragma once

#include <Windows.h>
#include <libloaderapi.h> // GetModuleHandleExA
#include <iostream>

static void Print(const char *pszFmt, ...)
{
	va_list va_alist;
	char szBuf[4096];
	va_start(va_alist, pszFmt);
	_vsnprintf(szBuf, sizeof(szBuf), pszFmt, va_alist);
	szBuf[sizeof(szBuf) - 1] = '\0';
	va_end(va_alist);

	printf(szBuf);
	fflush(stdout);
}

static inline void FailedToFind(const char *pszName)
{
	Print("Failed to find %s.\n", pszName);
}

static inline void FailedToPatch(const char *pszName)
{
	Print("Failed to patch %s.\n", pszName);
}

static inline void FailedToHook(const char *pszName)
{
	Print("Failed to hook %s.\n", pszName);
}

#ifdef DEBUG
	#define PRINTVAR(var)                        \
		{									     \
			HMODULE hModule = 0;                 \
			GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, (LPCSTR)var, (HMODULE *)&hModule); \
			Print("%s = %p (%p)\n", #var, (void *)((int)var - (int)hModule), hModule);                    \
		}

	#define PRINTVAREX(var) Print(__FUNCTION__ ": "); PRINTVAR(var)
#else
	#define PRINTVAR(var)
	#define PRINTVAREX(var)
#endif

#define PRINT(fmt, ...) Print(__FUNCTION__ ": "); Print(fmt, __VA_ARGS__)
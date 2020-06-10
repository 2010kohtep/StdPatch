#include <Windows.h>
#include <libloaderapi.h> // GetModuleHandleExA
#include <iostream>

void DbgTrace(const char *pszFmt, ...)
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

void FailedToFind(const char *pszName)
{
	DbgTrace("Failed to find %s.\n", pszName);
}

void FailedToPatch(const char *pszName)
{
	DbgTrace("Failed to patch %s.\n", pszName);
}

void FailedToHook(const char *pszName)
{
	DbgTrace("Failed to hook %s.\n", pszName);
}
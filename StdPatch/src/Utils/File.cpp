#include "precompiled.h"

bool FileExists(const char *pszName)
{
	return _access(pszName, 0) != -1;
}
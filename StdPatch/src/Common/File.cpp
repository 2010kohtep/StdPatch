#include <corecrt_io.h> // _access

bool FileExists(const char *pszName)
{
	return _access(pszName, 0) != -1;
}
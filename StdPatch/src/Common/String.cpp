#include <string.h>

int ExtractFileName(const char* pszSource, char* pszDest)
{
	if (pszSource == 0)
		return 0;

	char* p = (char*)pszSource;
	p = strchr(p, '\0');
	p = (char*)strrchr(pszSource, '\\');

	if (p == nullptr)
		return 0;

	strcpy(pszDest, p + 1);
	return strlen(pszDest);
}
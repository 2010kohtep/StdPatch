#include "common/Console.h"

void __cdecl SanityCheckVertexBoneLODFlags_MdlError(const char *pszMsg, ...)
{
	DbgTrace("Mismarked Bone flag, but screw this :^)\n");
}

int __cdecl IsInt24(int nValue)
{
#ifdef STDPATCH_OPTIMIZE
	if (nValue & 0xFF000000)
#else
	if (nValue < -0x800000 || nValue > 0x7FFFFF)
#endif
		DbgTrace(__FUNCTION__ ": Bad value = %d\n", nValue);

	return nValue;
}
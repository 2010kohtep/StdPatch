#include <Windows.h>

#include "main/Global.h"
#include "main/Funcs.h"

void FindModules()
{
	gStudioExe = new CModule("");
	gKernelDll = new CModule("KERNELBASE.dll");
}

bool Hook_MAXSTUDIOVERTS()
{
	ISearchPattern *pattern;
	void *addr;

	if (!g_pMAXSTUDIOVERTS)
		return false;

	pattern = gStudioExe->CreatePattern(addr);
	{
		const unsigned char abPattern[] = { 0x33, 0xFF, 0x68, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x68, 0xFF, 0xFF, 0xFF, 0xFF, 0x89 };

		pattern->FindPattern((void *)&abPattern, sizeof(abPattern), kPatternFlagsIgnoreFF);
		pattern->Transpose(3);
	}

	if (addr)
	{
		WritePrimitive<uint32_t>(addr, g_nMAXSTUDIOVERTS_NEW);
	}

	return (addr != nullptr);
}

bool Patch_SanityCheckVertexBoneLODFlags()
{
	ISearchPattern *pattern;
	void *addr;

	pattern = gStudioExe->CreatePattern(addr);
	{
		pattern->FindAnsiString("Mismarked Bone flag", kPatternFlagsStringPartial | kPatternFlagsStringRef);
		pattern->FindCall();
	}

	if (addr)
	{
		HookRegular(addr, SanityCheckVertexBoneLODFlags_MdlError);
	}

	return (addr != nullptr);
}

bool Find_IsInt24()
{
	ISearchPattern *pattern;

	pattern = gStudioExe->CreatePattern(g_pfnIsInt24);
	{
		pattern->FindAnsiString("int24 conversion out of range %d\n", kPatternFlagsStringRef);
		pattern->FindUInt16(0xEC8B, kPatternFlagsBack);
		pattern->Transpose(-1);
	}

	return (g_pfnIsInt24 != nullptr);
}

bool Find_BUFFERSIZE()
{
	ISearchPattern *pattern;

	pattern = gStudioExe->CreatePattern(g_pBUFFERSIZE);
	{
		const unsigned char abPattern[] = { 0x89, 0x85, 0xFF, 0xFF, 0xFF, 0xFF, 0x83, 0xBD, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x74 };

		pattern->FindPattern((void *)&abPattern, sizeof(abPattern), kPatternFlagsIgnoreFF);
		pattern->Transpose(sizeof(abPattern) + 2);
	}

	if (g_pBUFFERSIZE)
	{
		g_nBUFFERSIZE_DEF = *(int *)g_pBUFFERSIZE;
	}

	return (g_pBUFFERSIZE != nullptr);
}

bool Find_MAXFLEXCONTROLLER()
{
	ISearchPattern *pattern;

	pattern = gStudioExe->CreatePattern(g_pMAXFLEXCONTROLLER);
	{
		const unsigned char abPattern[] = { 0x81, 0xFA, 0x80, 0x00, 0x00, 0x00, 0x0F, 0x8D };

		pattern->FindPattern((void *)&abPattern, sizeof(abPattern));
		pattern->Transpose(2);
	}

	if (g_pMAXFLEXCONTROLLER)
	{
		g_nMAXFLEXCONTROLLER_NEW = *(int *)g_pMAXFLEXCONTROLLER;
	}

	return (g_pMAXFLEXCONTROLLER != nullptr);
}

bool Find_MAXSTUDIOVERTS()
{
	ISearchPattern *pattern;

	pattern = gStudioExe->CreatePattern(g_pMAXSTUDIOVERTS);
	{
		pattern->FindAnsiString("Too many unified vertices\n", kPatternFlagsStringRef);
		pattern->Transpose(-7);
	}

	if (g_pMAXSTUDIOVERTS)
		g_nMAXSTUDIOVERTS_DEF = *(int *)g_pMAXSTUDIOVERTS;

	return (g_pMAXSTUDIOVERTS != nullptr);
}

bool Find_VList()
{
	ISearchPattern *pattern;

	pattern = gStudioExe->CreatePattern(g_pVList);
	{
		pattern->ForceOutput(g_pfnAddToVlist);
		pattern->FindUInt16(0x048B);
		pattern->Transpose(3);
		pattern->Dereference();
	}

	return (g_pVList != nullptr);
}

bool Find_AddrToVlist()
{
	ISearchPattern *pattern;

	pattern = gStudioExe->CreatePattern(g_pfnAddToVlist);
	{
		pattern->FindAnsiString("Too many unified vertices\n", kPatternFlagsStringRef);
		pattern->FindUInt16(0xEC8B, kPatternFlagsBack);
		pattern->Transpose(-1);
	}

	return (g_pfnAddToVlist != nullptr);
}

bool Hook_VerticesPtrs()
{
	int hooks;

	g_VerticesPtrsNew.SetLength(g_nMAXSTUDIOVERTS_NEW);
	hooks = gStudioExe->HookRefAddr(g_pVList, g_VerticesPtrsNew.GetData(), 0x00);

	if (!hooks)
		g_VerticesPtrsNew.SetLength(0);

	return (hooks > 0);
}

bool Patch_WriteVTXFile()
{
	ISearchPattern *pattern;
	void *addr;

	pattern = gStudioExe->CreatePattern(addr);
	{
		pattern->ForceOutput(g_pBUFFERSIZE);
		pattern->Transpose(4);
		pattern->FindUInt32(g_nBUFFERSIZE_DEF);
	}

	if (addr)
	{
		WritePrimitive<uint32_t>(g_pBUFFERSIZE, g_nBUFFERSIZE_NEW);
		WritePrimitive<uint32_t>(addr, g_nBUFFERSIZE_NEW);
	}

	return (addr != nullptr);
}

bool Patch_MAXFLEXCONTROLLER()
{
	if (g_pMAXFLEXCONTROLLER)
	{
		WritePrimitive<uint32_t>(g_pMAXFLEXCONTROLLER, g_nMAXFLEXCONTROLLER_NEW);
		return true;
	}

	return false;
}

bool Hook_FlexController()
{
	ISearchPattern *pattern;
	void *addr;
	int hooks;

	pattern = gStudioExe->CreatePattern(addr);
	{
		const unsigned char abPattern[] = { 0x69, 0xC0, 0x08, 0x01, 0x00, 0x00, 0x68, 0x80, 0x00, 0x00, 0x00 };

		pattern->FindPattern((void *)&abPattern, sizeof(abPattern));
		pattern->Transpose(sizeof(abPattern));
		pattern->FindUInt8(0x05);
		pattern->Transpose(1);
	}

	g_FlexControllerNew.SetLength(g_nMAXFLEXCONTROLLER_NEW);
	addr = *(void **)addr;

	hooks = gStudioExe->HookRefAddr(addr, g_FlexControllerNew.GetData(), 0x00);
	if (!hooks)
		g_FlexControllerNew.SetLength(0);

	return (hooks > 0);
}

bool Hook_IsInt24()
{
	return gStudioExe->HookRefCall(g_pfnIsInt24, IsInt24);
}
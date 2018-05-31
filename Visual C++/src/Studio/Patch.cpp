#include "Patch.h"

#include <Utils/Memory.h>
#include <Studio/Global.h>
#include <Studio/Funcs.h>

#include <Common/Defines.h>
#include <Common/Console.h>

void FindModules()
{
	g_Base = TModule(GetModuleHandleA(NULL));
}

bool Hook_MAXSTUDIOVERTS()
{
	const unsigned char abPattern[] = { 0x33, 0xFF, 0x68, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x68, 0xFF, 0xFF, 0xFF, 0xFF, 0x89 };

	if (!g_pMAXSTUDIOVERTS)
		return false;

	if (auto pAddr = Memory::FindPattern(g_Base, abPattern, sizeof(abPattern), 3))
	{
		Memory::WriteLong(pAddr, g_nMAXSTUDIOVERTS_NEW);
		return true;
	}

	return false;
}

bool Patch_SanityCheckVertexBoneLODFlags()
{
	if (auto pAddr = Memory::FindPushOffset(g_Base, "Mismarked Bone flag"))
	{
		pAddr = Memory::SkipNextCall(pAddr, 1, 0, false);
		Memory::InsertCall(pAddr, SanityCheckVertexBoneLODFlags_MdlError);
		return true;
	}

	return false;
}

bool Find_IsInt24()
{
	if (auto pAddr = Memory::FindPushOffset(g_Base, "int24 conversion out of range %d"))
	{
		if (pAddr = Memory::FindWordPtr(pAddr, 64, 0xEC8B, -1, true))
		{
			g_pfnIsInt24 = (TIsInt24)pAddr;
			return true;
		}
	}

	return false;
}

bool Find_BUFFERSIZE()
{
	const unsigned char abPattern[] = { 0x89, 0x85, 0xFF, 0xFF, 0xFF, 0xFF, 0x83, 0xBD, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x74 };

	if (auto pAddr = Memory::FindPattern(g_Base, abPattern, sizeof(abPattern), sizeof(abPattern) + 1))
	{
		if (Memory::CheckByte(pAddr, 0x68, 0))
		{
			pAddr = Memory::Transpose(pAddr, 1);
			g_pBUFFERSIZE = pAddr;
			g_nBUFFERSIZE_DEF = *(int *)pAddr;
			return true;
		}
	}

	return false;
}

bool Find_MAXFLEXCONTROLLER()
{
	const unsigned char abPattern[] = { 0x81, 0xFA, 0x80, 0x00, 0x00, 0x00, 0x0F, 0x8D };

	if (auto pAddr = Memory::FindPattern(g_Base, abPattern, sizeof(abPattern), 2))
	{
		g_pMAXFLEXCONTROLLER = pAddr;
		g_nMAXFLEXCONTROLLER_NEW = *(int *)pAddr;
		return true;
	}

	return false;
}

bool Find_MAXSTUDIOVERTS()
{
	if (auto pAddr = Memory::FindPushOffset(g_Base, "Too many unified vertices"))
	{
		if (Memory::CheckByte(pAddr, 0x81, -12))
		{
			pAddr = Memory::Transpose(pAddr, -6);
			g_pMAXSTUDIOVERTS = pAddr;
			g_nMAXSTUDIOVERTS_DEF = *(int *)pAddr;
			return true;
		}
	}

	return false;
}

bool Find_VList()
{
	if (auto pAddr = (void *)g_pfnAddToVlist)
	{
		if (pAddr = Memory::FindWordPtr(pAddr, 64, 0x048B, 3, false))
		{
			g_pVList = *(void **)pAddr;
			return true;
		}
	}

	return false;
}

bool Find_AddrToVlist()
{
	if (auto pAddr = Memory::FindPushOffset(g_Base, "Too many unified vertices\n"))
	{
		if (pAddr = Memory::FindWordPtr(pAddr, 256, 0xEC8B, -1, true))
		{
			g_pfnAddToVlist = (TAddToVlist)pAddr;
			return true;
		}
	}

	return false;
}

bool Hook_VerticesPtrs()
{
	g_VerticesPtrsNew.SetLength(g_nMAXSTUDIOVERTS_NEW);

	auto nCount = Memory::HookRefAddr(g_Base, g_pVList, g_VerticesPtrsNew.GetData(), 0x00);

	if (nCount != 0)
		return true;

	g_VerticesPtrsNew.SetLength(0);
	return false;
}

bool Patch_WriteVTXFile()
{
	const unsigned char abPattern[] = { 0x89, 0x85, 0xFF, 0xFF, 0xFF, 0xFF, 0x83, 0xBD, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x74 };

	auto pAddr = Memory::Transpose(g_pBUFFERSIZE, sizeof(int));

	if (pAddr = Memory::FindLongPtr(pAddr, 64, g_nBUFFERSIZE_DEF, 0, false))
	{
		Memory::WriteLong(g_pBUFFERSIZE, g_nBUFFERSIZE_NEW);
		Memory::WriteLong(pAddr, g_nBUFFERSIZE_NEW);
		return true;
	}

	return false;
}

bool Patch_MAXFLEXCONTROLLER()
{
	if (g_pMAXFLEXCONTROLLER)
	{
		Memory::WriteLong(g_pMAXFLEXCONTROLLER, g_nMAXFLEXCONTROLLER_NEW);
		return true;
	}

	return false;
}

bool Hook_FlexController()
{
	const unsigned char abPattern[] = { 0x69, 0xC0, 0x08, 0x01, 0x00, 0x00, 0x68, 0x80, 0x00, 0x00, 0x00 };

	void *pAddr;
	pAddr = Memory::FindPattern(g_Base, abPattern, sizeof(abPattern), sizeof(abPattern));
	pAddr = Memory::FindBytePtr(pAddr, 128, 0x05, 1, false);

	if (pAddr)
	{
		g_FlexControllerNew.SetLength(g_nMAXFLEXCONTROLLER_NEW);

		pAddr = *(void **)pAddr;

		if (Memory::HookRefAddr(g_Base, pAddr, g_FlexControllerNew.GetData(), 0x00) != 0)
		{
			g_FlexControllerNew.SetLength(0);
			return false;
		}
		else
		{
			return true;
		}
	}

	return false;
}

bool Hook_IsInt24()
{
	return Memory::HookRefCall(g_Base, g_pfnIsInt24, IsInt24) != 0;
}
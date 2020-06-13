#include "common/Console.h"
#include "common/MemSearch.h"
#include "common/String.h"

#include "main/Global.h"

#include "Mods.h"

CModMgr gModMgr;

void CModMgr::RegisterMod(IMod *mod)
{
	m_Mods.push_back(mod);
}
void CModMgr::InitMods()
{
	for (auto &&mod : m_Mods)
	{
		if (!mod->Find())
		{
			DbgTrace("WARNING! Mod '%s' could not be applied - signature not found.\n");
			continue;
		}

		if (!mod->Patch())
		{
			DbgTrace("WARNING! Mod '%s' could not be applied - patch failed.\n");
			continue;
		}
	}
}

#pragma region Increase size of the array of vertices

class CMod_VList : public IMod
{
public:
	CMod_VList() : IMod() {}

	virtual const char *GetName()
	{
		return "CMod_VerticesPtrs";
	}

	virtual bool Find()
	{
		ISearchPattern *pattern;

		pattern = gStudioExe->CreatePattern(g_pfnAddToVlist);
		{
			pattern->FindAnsiString("Too many unified vertices\n", kPatternFlagsStringRef);
			pattern->FindUInt16(0xEC8B, kPatternFlagsBack);
			pattern->Transpose(-1);
		}

		if (!g_pfnAddToVlist)
			return false;

		pattern = gStudioExe->CreatePattern(g_pVList);
		{
			pattern->ForceOutput(g_pfnAddToVlist);
			pattern->FindUInt16(0x048B);
			pattern->Transpose(3);
			pattern->Dereference();
		}

		return (g_pVList != nullptr);
	}

	virtual bool Patch()
	{
		int hooks;

		if (!g_pVList)
			return false;

		g_VerticesPtrsNew.SetLength(g_nMAXSTUDIOVERTS_NEW);
		hooks = gStudioExe->HookRefAddr(g_pVList, g_VerticesPtrsNew.GetData(), 0x00);

		if (!hooks)
			g_VerticesPtrsNew.SetLength(0);

		return (hooks > 0);
	}
};

#pragma endregion

#pragma region Increase size of the array of weights

class CMod_MaxStudioVerts : public IMod
{
public:
	CMod_MaxStudioVerts() : IMod() {}

	virtual const char *GetName()
	{
		return "CMod_MaxStudioVerts";
	}

	virtual bool Find()
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

	virtual bool Patch()
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
};

#pragma endregion

#pragma region Suppress compiler crash in IsInt24 function

int __cdecl hkIsInt24(int nValue)
{
	if (nValue < -0x800000 || nValue > 0x7FFFFF)
		DbgTrace(__FUNCTION__ ": Bad value = %d\n", nValue);

	return nValue;
}

class CMod_IsInt24 : public IMod
{
public:
	CMod_IsInt24() : IMod() {}

	virtual const char *GetName()
	{
		return "CMod_IsInt24";
	}

	virtual bool Find()
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

	virtual bool Patch()
	{
		return gStudioExe->HookRefCall(g_pfnIsInt24, hkIsInt24);
	}
};

#pragma endregion

#pragma region Increase file buffer size in WriteVTXFile function

class CMod_WriteVTXFile : public IMod
{
public:
	CMod_WriteVTXFile() : IMod() {}

	virtual const char *GetName()
	{
		return "CMod_WriteVTXFile";
	}

	virtual bool Find()
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

	virtual bool Patch()
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
};

#pragma endregion

#pragma region Suppress "Mismarked Bone flag" error

void __cdecl hkSanityCheckVertexBoneLODFlags_MdlError(const char *pszMsg, ...)
{
	DbgTrace("Mismarked Bone flag, but screw this :^)\n");
}

class CMod_SanityCheckVertexBoneLODFlags : public IMod
{
public:
	CMod_SanityCheckVertexBoneLODFlags() : IMod() {}

	virtual const char *GetName()
	{
		return "CMod_SanityCheckVertexBoneLODFlags_MdlError";
	}

	virtual bool Find()
	{
		ISearchPattern *pattern;

		pattern = gStudioExe->CreatePattern(pfnSanityCheckVertexBoneLODFlags_MdlError);
		{
			pattern->FindAnsiString("Mismarked Bone flag", kPatternFlagsStringPartial | kPatternFlagsStringRef);
			pattern->FindCall();
		}

		return (pfnSanityCheckVertexBoneLODFlags_MdlError != nullptr);
	}

	virtual bool Patch()
	{
		if (!pfnSanityCheckVertexBoneLODFlags_MdlError)
			return false;

		HookRegular(pfnSanityCheckVertexBoneLODFlags_MdlError, hkSanityCheckVertexBoneLODFlags_MdlError);
		return true;
	}
};

#pragma endregion

#pragma region Increase size of the materials array

class CMod_MaterialsList : public IMod
{
public:
	CMod_MaterialsList() : IMod() {}

	virtual const char *GetName()
	{
		return "CMod_MaterialsList";
	}

	virtual bool Find()
	{
		ISearchPattern *pattern;

		pattern = gStudioExe->CreatePattern(g_pMaterialsList);
		{
			pattern->FindAnsiString("face %d references NULL texture %d\n", kPatternFlagsStringRef);
			pattern->FindCall(0, true, true);
			pattern->FindUInt16(0x868D);
			pattern->Transpose(2);
			pattern->Dereference();
		}

		if (!g_pMaterialsList)
			return false;

		pattern = gStudioExe->CreatePattern(g_pMaterialsListCheck);
		{
			pattern->FindAnsiString("Too many materials used, max %d\n", kPatternFlagsStringRef);
		}

		if (!g_pMaterialsListCheck)
			return false;

		pattern = gStudioExe->CreatePattern(g_pMaterialsIndex);
		{
			const unsigned char acPattern[] = { 0x8B, 0x85, 0xC8, 0xFE, 0xFF, 0xFF, 0x83, 0xC4, 0x10 };

			pattern->FindPattern((void *)&acPattern, sizeof(acPattern), kPatternFlagsIgnoreFF);
			pattern->FindUInt16(0x8D04);
			pattern->Transpose(2);
			pattern->Dereference();
		}

		if (!g_pMaterialsIndex)
			return false;

		g_nMAXMATERIALSCOUNT_DEF = *(uint8_t *)Transpose(g_pMaterialsListCheck, -2);

		return true;
	}

	virtual bool Patch()
	{
		if (!g_pMaterialsList || !g_pMaterialsListCheck)
			return false;

		g_MaterialsList.SetLength(g_nMAXMATERIALSCOUNT_NEW);

		if (gStudioExe->HookRefAddr(g_pMaterialsList, g_MaterialsList.GetData(), 0x00) == 0)
		{
			g_MaterialsList.SetLength(0);
			return false;
		}

		g_MaterialsIndex.SetLength(g_nMAXMATERIALSCOUNT_NEW);

		if (gStudioExe->HookRefAddr(g_pMaterialsIndex, g_MaterialsIndex.GetData(), 0x00) == 0)
		{
			g_MaterialsIndex.SetLength(0);
			return false;
		}

		WritePrimitive<uint8_t>(g_pMaterialsListCheck, g_nMAXMATERIALSCOUNT_NEW, -2);
		WritePrimitive<uint8_t>(g_pMaterialsListCheck, g_nMAXMATERIALSCOUNT_NEW, -6);
		return true;
	}
};

#pragma endregion

#pragma region Add vectored exception handler to catch new crashes

LONG NTAPI VectorExceptionFilter(_EXCEPTION_POINTERS *pException)
{
	//
	// Handle only 0xC0000000 exceptions
	//

	if (pException->ExceptionRecord->ExceptionCode >> 30 != 3)
		return EXCEPTION_CONTINUE_SEARCH;

	auto pRecord = pException->ExceptionRecord;
	auto pContext = pException->ContextRecord;

	auto pExceptionAddr = pRecord->ExceptionAddress;

	HMODULE hBase;
	GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, (LPCSTR)pRecord->ExceptionAddress, &hBase);

	char szBuf[MAX_PATH];
	GetModuleFileNameA(hBase, szBuf, sizeof(szBuf));
	ExtractFileName(szBuf, szBuf);

	DbgTrace("Exception happened and stdpatch catched it."
		"\n\t\tCode: 0x%p"
		"\n\t\tModule: %s"
		"\n\t\tBase: 0x%p"
		"\n\t\tAddress (A) - 0x%p"
		"\n\t\tAddress (R) - 0x%p"
		"\n",
		pRecord->ExceptionCode, szBuf, hBase, pExceptionAddr, (void *)((int)pExceptionAddr - (int)hBase));

	DbgTrace("EAX %.08X  EDX: %.08X  ECX: %.08X\nEDI: %.08X  ESI: %.08X\nEBX: %.08X  EBP: %.08X  ESP: %.08X  EIP: %.08X\n",
		pContext->Eax, pContext->Edx, pContext->Ecx,
		pContext->Edi, pContext->Esi,
		pContext->Ebx, pContext->Ebp, pContext->Esp,
		pContext->Eip);

	return EXCEPTION_EXECUTE_HANDLER;
}

bool __fastcall hkCStudioMDLApp_Create(void *that)
{
	auto bRes = g_pfnCStudioMDLApp_Create(that);

	AddVectoredExceptionHandler(1, VectorExceptionFilter);

	return bRes;
}

class CMod_ExceptionHandler : public IMod
{
private:
	void **m_VTable;

public:
	CMod_ExceptionHandler() : m_VTable(nullptr), IMod() {}

	virtual const char *GetName()
	{
		return "CMod_ExceptionHandler";
	}

	virtual bool Find()
	{
		gStudioExe->CreatePattern(m_VTable)->FindVTable("CStudioMDLApp");
		if (!m_VTable || !m_VTable[0])
			return false;

		g_pfnCStudioMDLApp_Create = decltype(g_pfnCStudioMDLApp_Create)(m_VTable[0]);
		return true;
	}

	virtual bool Patch()
	{
		if (!m_VTable)
			return false;

		WritePrimitive<void *>(&m_VTable[0], hkCStudioMDLApp_Create);
		return true;
	}
};

#pragma endregion

#pragma region Redirect OutputDebugStringA strings to compiler window

VOID WINAPI hkOutputDebugStringA(LPCSTR lpOutputString)
{
	DbgTrace(lpOutputString);
	g_gateOutputDebugStringA(lpOutputString);
}

class CMod_ExtendedLog : public IMod
{
public:
	CMod_ExtendedLog() : IMod() {}

	virtual const char *GetName()
	{
		return "CMod_ExtendedLog";
	}

	virtual bool Find()
	{
		return gKernelDll->GetLoaded();
	}

	virtual bool Patch()
	{
		g_gateOutputDebugStringA = decltype(g_gateOutputDebugStringA)(gKernelDll->HookExport("OutputDebugStringA", hkOutputDebugStringA));
		return g_gateOutputDebugStringA != nullptr;
	}
};

#pragma endregion

CMod_VList gMod_VList;
CMod_MaxStudioVerts gMod_MaxStudioVerts;
CMod_IsInt24 gMod_IsInt24;
CMod_WriteVTXFile gMod_WriteVTXFile;
CMod_SanityCheckVertexBoneLODFlags gMod_SanityCheckVertexBoneLODFlags;
CMod_ExceptionHandler gMod_ExceptionHandler;
CMod_MaterialsList gMod_MaterialsList;
CMod_ExtendedLog gMod_ExtendedLog;
#include <Windows.h>

#include "common/File.h"
#include "common/Console.h"

#include "main/Global.h"
#include "main/Patch.h"
#include "main/Exception.h"
#include "main/Detour.h"

void LoadConfig()
{
	char szBuf[MAX_PATH];
	GetModuleFileNameA(NULL, szBuf, sizeof(szBuf));
	
	auto pszSlash = strrchr(szBuf, '\\');
	if (pszSlash)
		*pszSlash = '\0';

	strcat_s(szBuf, "\\StdPatch.ini");

	if (FileExists(szBuf))
	{
		g_nMAXSTUDIOVERTS_NEW = GetPrivateProfileIntA("Main", "MaxStudioVerts", (1024 * 512), szBuf);
		g_nMAXMATERIALSCOUNT_NEW = GetPrivateProfileIntA("Main", "MaxMaterials", 127, szBuf);
		g_nBUFFERSIZE_NEW = GetPrivateProfileIntA("Main", "BufferSize", (1024 * 1024 * 32), szBuf);
		g_nMAXFLEXCONTROLLER_NEW = GetPrivateProfileIntA("Main", "FlexControllerSize", 400, szBuf);
	}
	else
	{
		DbgTrace("Warning! 'StdPatch.ini' configuration file could not be found, creating one with default values for patching.\n");

		WritePrivateProfileStringA("Main", "MaxStudioVerts", "524288", szBuf);
		WritePrivateProfileStringA("Main", "MaxMaterials", "127", szBuf);
		WritePrivateProfileStringA("Main", "BufferSize", "33554432", szBuf);
		WritePrivateProfileStringA("Main", "FlexControllerSize", "400", szBuf);
	}

	if (g_nMAXMATERIALSCOUNT_NEW > 127)
	{
		DbgTrace("Warning! Materials limit can't be more than 127, but received %d. Setting to 127...\n", g_nMAXMATERIALSCOUNT_NEW);
		g_nMAXMATERIALSCOUNT_NEW = 127;
	}
}

bool IsSFM()
{
	if (!gStudioExe->GetLoaded())
		return false;

	return GetVTableForClass(gStudioExe->GetBase(), gStudioExe->GetLastByte(), "CSFMBaseImporter") != nullptr;
}

VOID (WINAPI *orgOutputDebugStringA)(LPCSTR lpOutputString);

VOID WINAPI hkOutputDebugStringA(LPCSTR lpOutputString)
{
	DbgTrace(lpOutputString);

	orgOutputDebugStringA(lpOutputString);
}

void Hook_OutputDebugStringA()
{
	orgOutputDebugStringA = decltype(orgOutputDebugStringA)(gKernelDll->HookExport("OutputDebugStringA", hkOutputDebugStringA));
}

bool Find_MaterialsList()
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

bool Patch_MaterialsList()
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

void PatchStudioMdl()
{
	DbgTrace("StudioMdl Patcher 2.4.0 is started.\n");
	DbgTrace("Code by Alexander B. (2010kohtep) special for RED_EYE.\n");

	FindModules();

	if (!IsSFM())
	{
		MessageBoxA(HWND_DESKTOP, "StdPatch is available only for Source Filmmaker compiler. Aborting...", "Error", MB_SYSTEMMODAL);
		return;
	}

	InsertExceptionHandler();
	InsertDebugEvents();
	Hook_OutputDebugStringA();

	if (!Find_AddrToVlist())
		FailedToFind("AddrToVlist()");
	
	if (!Find_VList())
		FailedToFind("Vlist()");
	
	if (!Find_MAXSTUDIOVERTS())
		FailedToFind("MAXSTUDIOVERTS");
	else
	{
		if (!Hook_MAXSTUDIOVERTS())
			FailedToHook("MAXSTUDIOVERTS");
		else
		{
			if (!Hook_VerticesPtrs())
				FailedToHook("VerticesPtrs");
		}
	}

	if (!Find_IsInt24())
		FailedToFind("IsInt24()");
	else
	{
		if (!Hook_IsInt24())
			FailedToHook("IsInt24()");
	}
	
	if (!Find_BUFFERSIZE())
		FailedToFind("BUFFERSIZE");
	else
	{
		if (!Patch_WriteVTXFile())
			FailedToPatch("WriteVTXFile()");
	}

#if 0
	if (!Find_MAXFLEXCONTROLLER())
		FailedToFind("MAXFLEXCONTROLLER");

	if (!Patch_MAXFLEXCONTROLLER())
		FailedToPatch("MAXFLEXCONTROLLER");

	if (!Hook_FlexController())
		FailedToHook("FlexController");
#endif

	if (!Patch_SanityCheckVertexBoneLODFlags())
		FailedToFind("SanityCheckVertexBoneLODFlags()");

	if (!Find_MaterialsList())
		FailedToPatch("MaterialsList");
	else
	{
		if (!Patch_MaterialsList())
			FailedToHook("MaterialsList");
	}

	DbgTrace("\n");
}

BOOL WINAPI DllMain(_In_ HINSTANCE hinstDLL, _In_ DWORD fdwReason, _In_ LPVOID lpvReserved)
{
	UNREFERENCED_PARAMETER(hinstDLL);
	UNREFERENCED_PARAMETER(lpvReserved);

	if (fdwReason == DLL_PROCESS_ATTACH)
	{
		if (CreateMutexA(NULL, FALSE, "StdPatch") != (HANDLE)ERROR_ALREADY_EXISTS)
		{
			if (MessageBoxA(HWND_DESKTOP, "Enable stdpatch library functional?", "Question", 
				MB_YESNO | MB_ICONQUESTION | MB_SYSTEMMODAL) == IDYES)
			{
				LoadConfig();
				PatchStudioMdl();
			}
		}
	}

	return TRUE;
}
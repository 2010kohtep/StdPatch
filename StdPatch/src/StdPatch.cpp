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
		g_nBUFFERSIZE_NEW = GetPrivateProfileIntA("Main", "BufferSize", (1024 * 1024 * 32), szBuf);
		g_nMAXFLEXCONTROLLER_NEW = GetPrivateProfileIntA("Main", "FlexControllerSize", 400, szBuf);
	}
	else
	{
		DbgTrace("Warning! 'StdPatch.ini' configuration file could not be found, creating one with default values for patching.\n");

		WritePrivateProfileStringA("Main", "MaxStudioVerts", "524288", szBuf);
		WritePrivateProfileStringA("Main", "BufferSize", "33554432", szBuf);
		WritePrivateProfileStringA("Main", "FlexControllerSize", "400", szBuf);
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
void PatchStudioMdl()
{
	DbgTrace("StudioMdl Patcher 2.2.0 is started.\n");
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
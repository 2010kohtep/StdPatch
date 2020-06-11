#include <Windows.h>

#include "common/File.h"
#include "common/Console.h"

#include "main/Global.h"
#include "main/Detour.h"
#include "main/Mods.h"

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

void PatchStudioMdl()
{
	DbgTrace("StudioMdl Patcher 2.4.0 is started.\n");
	DbgTrace("Code by Alexander B. (2010kohtep) special for RED_EYE.\n");

	gStudioExe = new CModule("");
	gKernelDll = new CModule("KERNELBASE.dll");

	if (!IsSFM())
	{
		MessageBoxA(HWND_DESKTOP, "StdPatch is available only for Source Filmmaker compiler. Aborting...", "Error", MB_SYSTEMMODAL);
		return;
	}

	gModMgr.InitMods();
}

BOOL WINAPI DllMain(_In_ HINSTANCE hinstDLL, _In_ DWORD fdwReason, _In_ LPVOID lpvReserved)
{
	UNREFERENCED_PARAMETER(hinstDLL);
	UNREFERENCED_PARAMETER(lpvReserved);

	if (fdwReason == DLL_PROCESS_ATTACH)
	{
		if (CreateMutexA(NULL, FALSE, "StdPatch") == (HANDLE)ERROR_ALREADY_EXISTS)
			return TRUE;

		if (MessageBoxA(HWND_DESKTOP, "Enable stdpatch library functional?", "Question",
			MB_YESNO | MB_ICONQUESTION | MB_SYSTEMMODAL) != IDYES)
			return TRUE;

		LoadConfig();
		PatchStudioMdl();
	}

	return TRUE;
}
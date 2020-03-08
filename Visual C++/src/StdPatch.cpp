#include <Windows.h>
#include <iostream>

#include <Common/Console.h>
#include <Common/RTTI.h>

#include <Utils/Memory.h>
#include <Utils/String.h>
#include <Utils/File.h>

#include <Studio/Global.h>
#include <Studio/Patch.h>
#include <Studio/SDK.h>
#include <Studio/Funcs.h>
#include <Studio/Exception.h>
#include <Studio/Detour.h>

void LoadConfig()
{
	char szBuf[MAX_PATH];
	GetModuleFileNameA(NULL, szBuf, sizeof(szBuf));
	
	auto pszSlash = strrchr(szBuf, '\\');
	if (pszSlash)
		*pszSlash = '\0';

	strcat_s(szBuf, "\\stdpatch.ini");

	if (FileExists(szBuf))
	{
		g_nMAXSTUDIOVERTS_NEW = GetPrivateProfileIntA("Main", "MaxStudioVerts", 0x80000, szBuf);
		g_nBUFFERSIZE_NEW = GetPrivateProfileIntA("Main", "BufferSize", 2000000, szBuf);
		g_nMAXFLEXCONTROLLER_NEW = GetPrivateProfileIntA("Main", "FlexControllerSize", 400, szBuf);
	}
}

bool IsSFM()
{
	if (g_Base.pStart == nullptr)
		return false;

	return GetVTableForClass(g_Base, "CSFMBaseImporter") != nullptr;
}

void PatchStudioMdl()
{
	Print("StudioMdl Patcher 2.1.0 is started.\n");
	Print("Code by Alexander B. (2010kohtep) special for RED_EYE.\n");

	FindModules();

	if (!IsSFM())
	{
		MessageBoxA(HWND_DESKTOP, "StdPatch available only for Source Filmmaker compiler. Aborting...", "Error",
			MB_YESNO | MB_SYSTEMMODAL);

		return;
	}

	InsertExceptionHandler();
	InsertDebugEvents();

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

	Print("\n");
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
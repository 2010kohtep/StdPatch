#include "precompiled.h"

int main(int argc, char *argv[])
{
	HMODULE hKernel32;
	LPTHREAD_START_ROUTINE pfnLoadLibrary;
	STARTUPINFOA si;
	PROCESS_INFORMATION pi;
	void *pszRemoteLibName;
	char *pszFileDir;
	char *pszFileName;
	char szCmdLine[MAX_PATH];
	char szStudioMdlName[MAX_PATH];

	pszFileDir = _strdup(argv[0]);
	pszFileName = strrchr(pszFileDir, '\\');
	if (!pszFileName)
	{
		printf("Invalid current process file name!\n");
		return 0;
	}

	*pszFileName = '\0';

	sprintf_s(szStudioMdlName, "%s\\studiomdl.exe", pszFileDir);
	free(pszFileDir);

	if (_access(szStudioMdlName, 0) == -1)
	{
		printf("studiomdl.exe not found! Please, put injector in studiomdl.exe folder.\n");
		return 0;
	}

	hKernel32 = GetModuleHandleA("kernel32.dll");
	if (!hKernel32)
	{
		printf("Could not find kernel32.dll library! Code: %d\n", GetLastError());
		return 0;
	}
	
	pfnLoadLibrary = (LPTHREAD_START_ROUTINE)GetProcAddress(hKernel32, "LoadLibraryA");
	if (!pfnLoadLibrary)
	{
		printf("Could not find LoadLibraryA address! Code: %d\n", GetLastError());
		return 0;
	}

	memset(&si, 0, sizeof(si));
	memset(&pi, 0, sizeof(pi));
	si.cb = sizeof(si);
	si.dwFlags = STARTF_USESHOWWINDOW;
	si.wShowWindow = SW_SHOWNORMAL;

	char *pszCmdLine = GetCommandLineA();
	if (*pszCmdLine == '"')
	{
		pszCmdLine = strchr(++pszCmdLine, '"');
		pszCmdLine++;
	}
	else
	{
		pszCmdLine = strchr(pszCmdLine, ' ');
	}

	while (*pszCmdLine == ' ')
		pszCmdLine++;

	sprintf_s(szCmdLine, "%s %s", szStudioMdlName, pszCmdLine);

	if (!CreateProcessA(NULL, szCmdLine, NULL, NULL, FALSE, CREATE_DEFAULT_ERROR_MODE | CREATE_SUSPENDED, NULL, NULL, &si, &pi))
	{
		printf("Could not start studiomdl! Code: %d\n", GetLastError());
		return 0;
	}

	pszRemoteLibName = VirtualAllocEx(pi.hProcess, NULL, 4096, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
	if (!pszRemoteLibName)
	{
		printf("Could not allocate remote string! Code: %d\n", GetLastError());
		return 0;
	}

	char pszStdPatchname[] = "stdpatch.dll\0";
	if (!WriteProcessMemory(pi.hProcess, pszRemoteLibName, pszStdPatchname, sizeof(pszStdPatchname) + 1, NULL))
	{
		printf("Could not write remote string! Code: %d\n", GetLastError());
		return 0;
	}

	if (!CreateRemoteThread(pi.hProcess, NULL, 0, pfnLoadLibrary, pszRemoteLibName, 0, NULL))
	{
		printf("Could not create remote thread! Code: %d\n", GetLastError());
		return 0;
	}

	ResumeThread(pi.hThread);

	WaitForSingleObject(pi.hProcess, INFINITE);
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);

	return 0;
}
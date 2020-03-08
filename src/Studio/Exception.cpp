#include "precompiled.h"

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

	Print("Exception happened and stdpatch catched it."
		"\n\t\tCode: 0x%p"
		"\n\t\tModule: %s"
		"\n\t\tBase: 0x%p"
		"\n\t\tAddress (A) - 0x%p"
		"\n\t\tAddress (R) - 0x%p"
		"\n",
		pRecord->ExceptionCode, szBuf, hBase, pExceptionAddr, (void *)((int)pExceptionAddr - (int)hBase));

	return EXCEPTION_EXECUTE_HANDLER;
}

using TCStudioMDLAppCreate = bool(__fastcall *)(void *pSelf);
TCStudioMDLAppCreate orgCStudioMDLApp_Create;

bool __fastcall hkCStudioMDLApp_Create(void *pSelf)
{
	auto bRes = orgCStudioMDLApp_Create(pSelf);

	AddVectoredExceptionHandler(1, VectorExceptionFilter);

	return bRes;
}

void InsertExceptionHandler()
{
	auto pVMT = GetVTableForClass(g_Base, "CStudioMDLApp");
	orgCStudioMDLApp_Create = (TCStudioMDLAppCreate)pVMT[0];
	Memory::WritePointer(&pVMT[0], hkCStudioMDLApp_Create);
}
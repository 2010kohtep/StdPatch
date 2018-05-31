#pragma once

#ifndef _CRT_SECURE_NO_WARNINGS
	#define _CRT_SECURE_NO_WARNINGS
#endif

#include <Windows.h>

#define ASM_PUSH ((unsigned char)0x68)
#define ASM_CALL ((unsigned char)0xE8)
#define ASM_JUMP ((unsigned char)0xE9)

struct TModule
{
	// Module start address
	void *pStart;

	// Module end address
	void *pEnd;

	// Virtual size of module
	unsigned int uSize;

	TModule() : pStart(nullptr), pEnd(nullptr), uSize(0) {};

	TModule(void* pStart, unsigned int uSize);

	TModule(void *start);

	inline char* operator[](int iIndex) { return &((char *)pStart)[iIndex]; }
};

namespace Memory
{
	/* Very Basic Memory */

	bool IsValidPtr(void* pAddr);

	void *Absolute(void *addr);
	void *Transpose(void *addr, int offset, bool bDeref);
	void *Transpose(void *addr, int offset);
	PVOID Relative(PVOID NewFunc, PVOID Address);

	/* Basic Memory */

	bool Copy(PBYTE dest, PBYTE source, INT size);

	/* Modules */

	int GetModuleSize(PVOID addr);
	PIMAGE_SECTION_HEADER GetSectionPtr(HMODULE module, char *name);
	DWORD GetSectionSize(HMODULE module, char *name);

	/* Memory Protection */

	DWORD SetProtect(PVOID addr, DWORD protect, int size);

	/* Write Memory */

	PVOID WriteInt64(PVOID addr, __int64 value);
	PVOID WriteCStr(PVOID addr, char* value);
	PVOID WriteBuffer(PVOID addr, PVOID buffer, DWORD size);
	PVOID WriteDouble(PVOID addr, double value);
	PVOID WriteLong(PVOID addr, DWORD value);
	PVOID WritePointer(PVOID addr, PVOID value);
	PVOID WriteFloat(PVOID addr, float value);
	PVOID WriteWord(PVOID addr, WORD value);
	PVOID WriteByte(PVOID addr, BYTE value);

	/* Detours */

	void InsertFunc(PVOID addrSource, PVOID addrDest, bool isCall);
	void InsertJump(PVOID addr, PVOID jmp);
	void InsertCall(PVOID addr, PVOID call);

	/* Search Memory */

	PVOID FindBytePtr(TModule& module, BYTE value, int offset, bool back);
	PVOID FindWordPtr(TModule& module, WORD value, int offset, bool back);
	PVOID FindLongPtr(TModule& module, DWORD value, int offset, bool back);
	PVOID FindLongPtr(PVOID pAddr, PVOID pEnd, DWORD value, int offset, bool back);
	PVOID FindBytePtr(PVOID pAddr, DWORD uSize, BYTE value, int offset, bool back);
	PVOID FindWordPtr(PVOID pAddr, DWORD uSize, WORD value, int offset, bool back);
	PVOID FindLongPtr(PVOID pAddr, DWORD uSize, DWORD value, int offset, bool back);

	PVOID FindNextAddr(PVOID start, int offset, bool back, BYTE hdr);
	PVOID FindNextAddrEx(PVOID start, bool back, BYTE hdr);

	PVOID SkipNextAddr(PVOID start, DWORD number, int offset, bool back, BYTE hdr);
	PVOID SkipNextAddrEx(PVOID start, DWORD number, bool back, BYTE hdr);
	PVOID FindNextCall(PVOID start, int offset, bool back);
	PVOID FindNextCallEx(PVOID start, bool back);
	PVOID SkipNextCall(PVOID start, DWORD number, int offset, bool back);
	PVOID SkipNextCallEx(PVOID start, DWORD number, bool back);

	PVOID FindNextJump(PVOID start, int offset, bool back);
	PVOID FindNextJumpEx(PVOID start, bool back);
	PVOID SkipNextJump(PVOID start, DWORD position, int offset, bool back);
	PVOID SkipNextJumpEx(PVOID start, DWORD position, bool back);

	PVOID FindPattern(TModule& module, PVOID pattern, DWORD patternSize, int offset);

	template <typename T> PVOID FindPattern(TModule& module, T pattern, DWORD patternSize, int offset)
	{
		return FindPattern(module, (PVOID)pattern, patternSize, offset);
	}

	PVOID FindPushOffset(TModule& module, const char *szStr);

	PVOID FindRefAddr(void *pStart, void *pEnd, void *reference, byte header);
	PVOID FindRefAddr(TModule& module, PVOID addr);
	PVOID FindRefCall(TModule& module, PVOID addr);
	PVOID FindRefJump(TModule& module, PVOID addr);
	DWORD HookRefAddr(TModule& module, PVOID oldFunc, PVOID newFunc, BYTE hdr);
	DWORD HookRefCall(TModule& module, PVOID oldFunc, PVOID newFunc);
	DWORD HookRefJump(TModule& module, PVOID oldFunc, PVOID newFunc);

	/* Check Memory */

	bool CheckByte(PVOID addr, BYTE value, int offset);
	bool CheckWord(PVOID addr, WORD value, int offset);
	bool CheckLong(PVOID addr, DWORD value, int offset);

	bool Bounds(TModule& module, void *addr);

	/* Detours */

	PVOID HookRegular(PVOID oldFunc, PVOID newFunc, INT codeSize);
	PVOID HookWinAPI(PVOID oldFunc, PVOID newFunc);
	PVOID HookNativeAPI(PVOID oldFunc, PVOID newFunc);
}
#include <Windows.h>

#include "Memory.h"

#ifndef _CRT_SECURE_NO_WARNINGS
	#define _CRT_SECURE_NO_WARNINGS
#endif

const BYTE ASM_PUSH = 0x68;
const BYTE ASM_CALL = 0xE8;
const BYTE ASM_JUMP = 0xE9;

namespace Memory
{
	inline bool IsValidPtr(void* pAddr)
	{
		return pAddr != nullptr;
	}

	/* Very Basic Memory */

	inline void *Absolute(void *addr)
	{
		return (void *)((int)addr + *(int *)addr + sizeof(addr));
	}

	inline void *Transpose(void *addr, int offset, bool bDeref)
	{
		return bDeref ? *(void **)((int)addr + offset) : (void *)((int)addr + offset);
	}

	inline void *Transpose(void *addr, int offset)
	{
		return Transpose(addr, offset, false);
	}

	inline PVOID Relative(PVOID NewFunc, PVOID Address)
	{
		return (PVOID)((DWORD)NewFunc - (DWORD)Address - sizeof(DWORD));
	}

	/* Basic Memory */
	bool Copy(PBYTE dest, PBYTE source, INT size)
	{
		if (!IsValidPtr(dest) || !IsValidPtr(source) || size <= 0)
			return false;

		memcpy(dest, source, size);

		// while (size > 0)
		// {
		// 	switch (size % sizeof(INT))
		// 	{
		// 	case 4: dest[0] = source[0];
		// 	case 3: dest[1] = source[1];
		// 	case 2: dest[2] = source[2];
		// 	case 1: dest[3] = source[3];
		// 	}
		// 
		// 	dest += sizeof(INT);
		// 	source += sizeof(INT);
		// 	size -= sizeof(INT);
		// }

		return true;
	}

	/* Modules */
	int GetModuleSize(PVOID addr)
	{
		if (!IsValidPtr(addr))
			return 0;

		PIMAGE_DOS_HEADER dos = (PIMAGE_DOS_HEADER)addr;
		PIMAGE_NT_HEADERS nt = (PIMAGE_NT_HEADERS)((DWORD)dos + dos->e_lfanew);

		return nt->OptionalHeader.SizeOfImage;
	}

	PIMAGE_SECTION_HEADER GetSectionPtr(HMODULE module, char *name)
	{
		if ((module == nullptr) || (name == nullptr) || (*name == '\0'))
			return nullptr;

		PIMAGE_DOS_HEADER dos = (PIMAGE_DOS_HEADER)module;
		PIMAGE_NT_HEADERS nt = (PIMAGE_NT_HEADERS)((DWORD)dos + dos->e_lfanew);

		PIMAGE_SECTION_HEADER sections = IMAGE_FIRST_SECTION(nt);

		for (int i = 0; i < nt->FileHeader.NumberOfSections; i++)
		{
			if (!strcmp((char *)sections[i].Name, name))
			{
				return &sections[i];
			}
		}

		return nullptr;
	}

	DWORD GetSectionSize(HMODULE module, char *name)
	{
		PIMAGE_SECTION_HEADER section = GetSectionPtr(module, name);

		if (section == nullptr)
			return 0;

		return section->SizeOfRawData;
	}

	/* Memory Protection */
	DWORD SetProtect(PVOID addr, DWORD protect, int size)
	{
		DWORD result;
		VirtualProtect(addr, size, protect, &result);

		return result;
	}

	/* Write Memory */
	PVOID WriteInt64(PVOID addr, __int64 value)
	{
		if (!addr)
		{
			return nullptr;
		}

		DWORD protect = SetProtect(addr, PAGE_EXECUTE_READWRITE, sizeof(value));
		*(__int64 *)addr = value;
		SetProtect(addr, protect, sizeof(value));
		return Transpose(addr, sizeof(value), false);
	}

	PVOID WriteCStr(PVOID addr, char* value)
	{
		if ((addr == nullptr) || (value == nullptr))
			return nullptr;

		int length = strlen(value) + sizeof('\0');

		DWORD old = SetProtect(addr, PAGE_EXECUTE_READWRITE, length);
		strcpy((PCHAR)addr, value);
		SetProtect(addr, old, length);

		return (PVOID)((DWORD)addr + length);
	}

	PVOID WriteBuffer(PVOID addr, PVOID buffer, DWORD size)
	{
		if ((addr == nullptr) || (buffer == nullptr) || (size <= 0))
			return nullptr;

		DWORD old = SetProtect(addr, PAGE_EXECUTE_READWRITE, size);
		Copy((PBYTE)addr, (PBYTE)buffer, size);
		SetProtect(addr, old, size);

		return (PVOID)((DWORD)addr + size);
	}

	PVOID WriteDouble(PVOID addr, double value)
	{
		if (addr == nullptr)
			return nullptr;

		DWORD old = SetProtect(addr, PAGE_EXECUTE_READWRITE, sizeof(value));
		Copy((PBYTE)addr, (PBYTE)&value, sizeof(value));
		SetProtect(addr, old, sizeof(value));

		return (PVOID)((DWORD)addr + sizeof(value));
	}

	PVOID WriteLong(PVOID addr, DWORD value)
	{
		if (addr == nullptr)
			return nullptr;

		DWORD old = SetProtect(addr, PAGE_EXECUTE_READWRITE, sizeof(value));
		*(DWORD*)addr = value;
		SetProtect(addr, old, sizeof(value));

		return (PVOID)((DWORD)addr + sizeof(value));
	}

	PVOID WritePointer(PVOID addr, PVOID value)
	{
		return WriteLong(addr, (DWORD)value);
	}

	PVOID WriteFloat(PVOID addr, float value)
	{
		return WriteLong(addr, (DWORD)value);
	}

	PVOID WriteWord(PVOID addr, WORD value)
	{
		if (addr == nullptr)
			return nullptr;

		DWORD Old = SetProtect(addr, PAGE_EXECUTE_READWRITE, sizeof(value));
		*(PWORD)addr = value;
		SetProtect(addr, Old, sizeof(value));

		return (PVOID)((DWORD)addr + sizeof(value));
	}

	PVOID WriteByte(PVOID addr, BYTE value)
	{
		if (addr == nullptr)
			return nullptr;

		DWORD Old = SetProtect(addr, PAGE_EXECUTE_READWRITE, sizeof(value));
		*(PBYTE)addr = value;
		SetProtect(addr, Old, sizeof(value));

		return (PVOID)((DWORD)addr + sizeof(value));
	}

	/* Detours */
	void InsertFunc(PVOID addrSource, PVOID addrDest, bool isCall)
	{
		if ((addrSource == nullptr) || (addrDest == nullptr))
			return;

		BYTE b = isCall ? ASM_CALL : ASM_JUMP;

		addrSource = WriteByte(addrSource, b);
		PVOID p = Relative(addrDest, addrSource);

		WritePointer(addrSource, p);

		return;
	}

	void InsertJump(PVOID addr, PVOID jmp)
	{
		return InsertFunc(addr, jmp, FALSE);
	}

	void InsertCall(PVOID addr, PVOID call)
	{
		return InsertFunc(addr, call, TRUE);
	}

	/* Search Memory */

	__forceinline DWORD __fastcall Inc(DWORD X) { return X + 1; }
	__forceinline DWORD __fastcall Dec(DWORD X) { return X - 1; }

	PVOID FindBytePtr(TModule& module, BYTE value, int offset, bool back)
	{
		DWORD p = (DWORD)module.pStart;
		DWORD(__fastcall *f)(DWORD X) = !back ? &Inc : &Dec;

		INT size = module.uSize;
		PVOID addr = &((BYTE *)p)[size];

		if (IsValidPtr((FARPROC)addr))
		{
			size -= sizeof(value);
		}

		if (size > sizeof(value))
		{
			size -= sizeof(value);
		}

		while (size > 0)
		{
			if (*(PBYTE)p == value)
				return (PVOID)((DWORD)p + offset);

			p = f(p);
			size--;
		}

		return nullptr;
	}

	PVOID FindWordPtr(TModule& module, WORD value, int offset, bool back)
	{
		DWORD p = (DWORD)module.pStart;
		DWORD(__fastcall *f)(DWORD X) = !back ? &Inc : &Dec;

		INT size = module.uSize;
		PVOID addr = &((BYTE *)p)[size];

		if (IsValidPtr((FARPROC)addr))
		{
			size -= sizeof(value);
		}

		if (size > sizeof(value))
		{
			size -= sizeof(value);
		}

		while (size > 0)
		{
			if (*(PWORD)p == value)
				return (PVOID)((DWORD)p + offset);

			p = f(p);
			size--;
		}

		return nullptr;
	}

	PVOID FindLongPtr(TModule& module, DWORD value, int offset, bool back)
	{
		DWORD p = (DWORD)module.pStart;
		DWORD(__fastcall *f)(DWORD X) = !back ? &Inc : &Dec;

		INT size = module.uSize;
		PVOID addr = &((BYTE *)p)[size];

		if (IsValidPtr((FARPROC)addr))
		{
			size -= sizeof(DWORD);
		}

		if (size > sizeof(DWORD))
		{
			size -= sizeof(DWORD);
		}

		while (size > 0)
		{
			if (*(PDWORD)p == value)
				return (PVOID)((DWORD)p + offset);

			p = f(p);
			size--;
		}

		return nullptr;
	}

	PVOID FindNextAddr(PVOID start, int offset, bool back, BYTE hdr)
	{
		if (start == nullptr)
			return nullptr;

		DWORD p = (DWORD)start;
		DWORD(__fastcall *f)(DWORD X) = !back ? &Inc : &Dec;

		_MEMORY_BASIC_INFORMATION mem;
		if (!VirtualQuery((PVOID)p, &mem, sizeof(mem)))
			return nullptr;

		PVOID baseAddr = mem.AllocationBase;

		do
		{
			while (*(PBYTE)p != hdr)
			{
				p = f(p);

				if (p == (DWORD)Transpose(baseAddr, -1))
					return nullptr;
			}

			PVOID aaddr = Absolute(PVOID((DWORD)p + 1));
			if (VirtualQuery(aaddr, &mem, sizeof(mem)))
			{
				if (mem.AllocationBase == baseAddr)
					break;
				else
					p = f(p);
			}
			else
				p = f(p);
		} while (true);

		return (PVOID)(p + offset);
	}

	PVOID FindNextAddrEx(PVOID start, bool back, BYTE hdr)
	{
		PVOID result = FindNextAddr(start, 1, back, hdr);

		if (result)
			result = Absolute(result);

		return result;
	}

	PVOID FindBytePtr(PVOID pAddr, DWORD uSize, BYTE value, int offset, bool back)
	{
		TModule module = TModule(pAddr, uSize);
		return FindBytePtr(module, value, offset, back);
	}

	PVOID FindWordPtr(PVOID pAddr, DWORD uSize, WORD value, int offset, bool back)
	{
		TModule module = TModule(pAddr, uSize);
		return FindWordPtr(module, value, offset, back);
	}

	PVOID FindLongPtr(PVOID pAddr, DWORD uSize, DWORD value, int offset, bool back)
	{
		TModule module = TModule(pAddr, uSize);
		return FindLongPtr(module, value, offset, back);
	}

	PVOID FindLongPtr(PVOID pAddr, PVOID pEnd, DWORD value, int offset, bool back)
	{
		TModule module = TModule(pAddr, (DWORD)((DWORD)pEnd - (DWORD)pAddr));
		return FindLongPtr(module, value, offset, back);
	}

	PVOID SkipNextAddr(PVOID start, DWORD number, int offset, bool back, BYTE hdr)
	{
		if ((!start) || (number <= 0))
			return nullptr;

		PVOID result = start;
		DWORD offsetto = back ? -1 : 1;
		DWORD(__fastcall *f)(DWORD X) = !back ? &Dec : &Inc;

		do
		{
			result = FindNextAddr(result, offsetto, back, hdr);
			if (!result)
				return nullptr;

			number--;
			if (!number)
			{
				result = (PVOID)f((DWORD)result);
				return (PVOID)((DWORD)result + offset);
			}
		} while (true);
	}

	PVOID SkipNextAddrEx(PVOID start, DWORD number, bool back, BYTE hdr)
	{
		PVOID result = SkipNextAddr(start, number, 1, back, hdr);

		if (result)
			result = Absolute(result);

		return result;
	}

	PVOID FindNextCall(PVOID start, int offset, bool back)
	{
		return FindNextAddr(start, offset, back, ASM_CALL);
	}

	PVOID FindNextCallEx(PVOID start, bool back)
	{
		return FindNextAddrEx(start, back, ASM_CALL);
	}

	PVOID SkipNextCall(PVOID start, DWORD number, int offset, bool back)
	{
		return SkipNextAddr(start, number, offset, back, ASM_CALL);
	}

	PVOID SkipNextCallEx(PVOID start, DWORD number, bool back)
	{
		return SkipNextAddrEx(start, number, back, ASM_CALL);
	}

	PVOID FindNextJump(PVOID start, int offset, bool back)
	{
		return FindNextAddr(start, offset, back, ASM_JUMP);
	}

	PVOID FindNextJumpEx(PVOID start, bool back)
	{
		return FindNextAddrEx(start, back, ASM_JUMP);
	}

	PVOID SkipNextJump(PVOID start, DWORD position, int offset, bool back)
	{
		return SkipNextAddr(start, position, offset, back, ASM_JUMP);
	}

	PVOID SkipNextJumpEx(PVOID start, DWORD position, bool back)
	{
		return SkipNextAddrEx(start, position, back, ASM_JUMP);
	}

	bool CompareMemory(void *dest, void *src, int len)
	{
		for (int i = 0; i < len; i++)
		{
			BYTE destbyte = ((PBYTE)dest)[i];
			BYTE srcbyte = ((PBYTE)src)[i];

			if (srcbyte != 0xFF)
			{
				if (destbyte != srcbyte)
					return false;
			}
		}

		return true;
	}

	PVOID FindPattern(TModule &rModule, PVOID pPattern, DWORD nPatternSize, int nOffset)
	{
		auto pAddrCur = (unsigned char *)rModule.pStart;
		auto pAddrEnd = (unsigned char *)rModule.pEnd;

		while (pAddrCur != pAddrEnd)
		{
			auto p1 = pAddrCur;
			auto p2 = (unsigned char *)pPattern;

			if ((*p1 == *p2) && CompareMemory(p1, p2, nPatternSize))
				return Transpose(p1, nOffset);

			pAddrCur++;
		}

		return nullptr;
	}

	PVOID FindPushOffset(TModule& module, const char *szStr)
	{
		if ((module.pStart == nullptr) || (module.uSize <= 0) || (szStr == nullptr) || (*szStr == '\0'))
			return nullptr;

		PVOID p = FindPattern(module, szStr, strlen(szStr), 0);
		if (p == nullptr)
			return nullptr;

		BYTE a[5] = { ASM_PUSH, 0x00, 0x00, 0x00, 0x00 };
		*(PVOID *)&a[1] = p;

		return FindPattern(module, a, sizeof(a), 0);
	}

	PVOID FindRefAddr(void *pStart, void *pEnd, void *reference, BYTE header = 0x00)
	{
		if (!pStart)
			return nullptr;

		if (!reference)
			return nullptr;

		auto pAddr = (BYTE *)pStart;
		pEnd = Transpose(pEnd, -5);

		if (header == 0x00)
		{
			while (pAddr < pEnd)
			{
				if (*(void **)pAddr == reference)
					return pAddr;

				pAddr++;
			}
		}
		else
		{
			while (pAddr < pEnd)
			{
				if (*pAddr == header)
				{
					if (Absolute(Transpose(pAddr, 1)) == reference)
						return pAddr;
				}

				pAddr++;
			}
		}

		return nullptr;
	}

	PVOID FindRefAddr(TModule& module, PVOID addr)
	{
		return FindRefAddr(module.pStart, module.pEnd, addr, 0x00);
	}

	PVOID FindRefCall(TModule& module, PVOID addr)
	{
		return FindRefAddr(module.pStart, module.pEnd, addr, ASM_CALL);
	}

	PVOID FindRefJump(TModule& module, PVOID addr)
	{
		return FindRefAddr(module.pStart, module.pEnd, addr, ASM_JUMP);
	}

	DWORD HookRefAddr(TModule& rModule, PVOID pReference, PVOID pNewReference, BYTE nHdr)
	{
		int nHookCount = 0;

		if ((rModule.pStart == nullptr) || (rModule.uSize <= 0) || (pReference == nullptr) || (pNewReference == nullptr))
			return nHookCount;

		auto pCurAddr = rModule.pStart;

		while (true)
		{
			pCurAddr = FindRefAddr(pCurAddr, rModule.pEnd, pReference, nHdr);

			if (pCurAddr == nullptr)
				break;

			if (nHdr != 0x00)
				InsertFunc(pCurAddr, pNewReference, nHdr == ASM_CALL);
			else
				WritePointer(pCurAddr, pNewReference);
	
			nHookCount++;
		}

		return nHookCount;
	}

	DWORD HookRefCall(TModule& module, PVOID oldFunc, PVOID newFunc)
	{
		return HookRefAddr(module, oldFunc, newFunc, ASM_CALL);
	}

	DWORD HookRefJump(TModule& module, PVOID oldFunc, PVOID newFunc)
	{
		return HookRefAddr(module, oldFunc, newFunc, ASM_JUMP);
	}

	/* Check Memory */

	bool CheckByte(PVOID addr, BYTE value, int offset)
	{
		if (addr == nullptr)
			return false;

		return *(BYTE *)((int)addr + offset) == value;
	}

	bool CheckWord(PVOID addr, WORD value, int offset)
	{
		if (addr == nullptr)
			return false;

		return *(WORD *)((int)addr + offset) == value;
	}

	bool CheckLong(PVOID addr, DWORD value, int offset)
	{
		if (addr == nullptr)
			return false;

		return *(DWORD *)((int)addr + offset) == value;
	}

	bool Bounds(TModule& module, void *addr)
	{
		return !((int)addr < (int)module.pStart) || ((int)addr >(int)module.pEnd);
	}

	PVOID AllocExecutableMem(int size)
	{
		auto pAddr = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY | HEAP_CREATE_ALIGN_16, size);

		unsigned long uOldProtect;
		VirtualProtect(pAddr, size, PAGE_EXECUTE_READWRITE, &uOldProtect);

		return pAddr;
	}

	/* Detours */

	PVOID HookRegular(PVOID oldFunc, PVOID newFunc, INT codeSize)
	{
		if (!oldFunc || !newFunc)
			return nullptr;

		PVOID result = AllocExecutableMem(codeSize + 5);
		Copy((PBYTE)result, (PBYTE)oldFunc, codeSize);
		InsertJump(Transpose(result, codeSize), Transpose(oldFunc, codeSize));
		InsertJump(oldFunc, newFunc);

		return result;
	}

	PVOID HookWinAPI(PVOID oldFunc, PVOID newFunc)
	{
		if (!oldFunc || !newFunc)
			return nullptr;

		if (!CheckWord(oldFunc, 0xFF89, 0))
			return nullptr;

		return HookRegular(oldFunc, newFunc, 5);
	}

	PVOID HookNativeAPI(PVOID oldFunc, PVOID newFunc)
	{
		if (!oldFunc || !newFunc)
			return nullptr;

		if (!CheckByte(oldFunc, 0xB8, 0))
			return nullptr;

		return HookRegular(oldFunc, newFunc, 5);
	}
}

TModule::TModule(void* pStart, unsigned int uSize)
{
	this->pStart = pStart;
	this->uSize = uSize;
	this->pEnd = (void *)((unsigned int)pStart + uSize - 1);
}

TModule::TModule(void *pStart)
{
	this->pStart = pStart;
	this->uSize = Memory::GetModuleSize(pStart);
	this->pEnd = (void *)((unsigned int)pStart + this->uSize - 1);
}
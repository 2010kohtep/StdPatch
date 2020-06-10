#include <Windows.h>
#include <cstdint>
#include <stddef.h>

#include "MemTools.h"
#include "public/opcode_len_calc.h"

#pragma region Memory Management

bool FreeMemory(void *data)
{
	return HeapFree(GetProcessHeap(), 0, data) != FALSE;
}

void *GetMem(size_t size)
{
	return HeapAlloc(GetProcessHeap(), 0, size);
}

void *GetExecMem(size_t size)
{
	DWORD oldProtect;
	void *ret;

	ret = GetMem(size);
	if (!ret)
		return nullptr;

	if (!VirtualProtect(ret, size, PAGE_EXECUTE_READWRITE, &oldProtect))
	{
		MEM_TRACE(__FUNCTION__ ": Memory allocated at %X, but VirtualProtect returned %X.", GetLastError());
		FreeMemory(ret);
		return nullptr;
	}

	return ret;
}

bool ValidateMemory(void *addr)
{
	MEMORY_BASIC_INFORMATION mem;

	if (!addr)
	{
		MEM_TRACE(__FUNCTION__ ": Invalid address.");
		return false;
	}

	if (VirtualQuery(addr, &mem, sizeof(mem)) != sizeof(mem))
	{
		MEM_TRACE(__FUNCTION__ ": VirtualQuery returned %X.", GetLastError());
		return false;
	}

	if (mem.Protect == 0 || mem.Protect == PAGE_NOACCESS)
	{
		MEM_TRACE(__FUNCTION__ ": Invalid memory protection flag - %d.", mem.Protect);
		return false;
	}

	return true;
}

bool IsExecutable(void *addr)
{
	MEMORY_BASIC_INFORMATION mem;

	if (!addr)
	{
		MEM_TRACE(__FUNCTION__ ": Invalid address.");
		return false;
	}

	if (VirtualQuery(addr, &mem, sizeof(mem)) != sizeof(mem))
	{
		MEM_TRACE(__FUNCTION__ ": VirtualQuery returned %X.", GetLastError());
		return false;
	}

	if (mem.Protect == 0 || mem.Protect == PAGE_NOACCESS)
	{
		MEM_TRACE(__FUNCTION__ ": Invalid memory protection flag - %d.", mem.Protect);
		return false;
	}

	switch (mem.Protect)
	{
	case PAGE_EXECUTE:
	case PAGE_EXECUTE_READ:
	case PAGE_EXECUTE_READWRITE:
	case PAGE_EXECUTE_WRITECOPY:
		return true;

	default:
		return false;
	}
}

void *GetBaseAddr(void *addr)
{
	HMODULE h;

	if (!addr)
	{
		MEM_TRACE(__FUNCTION__ ": Invalid address.");
		return nullptr;
	}

	if (GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, (LPCSTR)addr, &h))
		return (void *)h;
	else
		return nullptr;
}

unsigned int GetModuleSize(void *base)
{
	PIMAGE_DOS_HEADER dos;
	PIMAGE_NT_HEADERS nt;

	if (!base)
	{
		MEM_TRACE(__FUNCTION__ ": Invalid base.");
		return 0;
	}

	dos = (PIMAGE_DOS_HEADER)base;
	nt = (PIMAGE_NT_HEADERS)(LONG(dos) + dos->e_lfanew);
	return nt->OptionalHeader.SizeOfImage;
}

#pragma endregion

#pragma region Memory Basic

void *Transpose(void *addr, int offset)
{
	return (addr) ? (void *)((int)addr + offset) : nullptr;
}

void *Absolute(void *address, int offset = 0)
{
	if (!address)
	{
		MEM_TRACE(__FUNCTION__ ": Invalid address.");
		return nullptr;
	}

	address = Transpose(address, offset);

	return (void *)((int)address + *(int *)address + sizeof address);
}

void *Relative(void *to, void *address)
{
	return Transpose(address, -(int)to - sizeof(void *));
}

bool Bounds(void *address, void *bottom, void *top)
{
	return ((size_t)address >= (size_t)bottom) && ((size_t)address <= (size_t)top);
}

#pragma endregion

#pragma region Memory Read

bool ReadData(void *dest, void *source, size_t count)
{
	if (!source)
	{
		MEM_TRACE(__FUNCTION__ ": Invalid source address.");
		return false;
	}

	if (!dest)
	{
		MEM_TRACE(__FUNCTION__ ": Invalid dest address.");
		return false;
	}

	if (!count)
	{
		MEM_TRACE(__FUNCTION__ ": Invalid data count.");
		return false;
	}

	memcpy(dest, source, count);
	return true;
}

#pragma endregion

#pragma region Memory Write

bool WriteData(void *dest, void *source, size_t size, int flags)
{
	DWORD oldProtect;

	if (!source)
	{
		MEM_TRACE(__FUNCTION__ ": source is null.");
		return false;
	}

	if (!dest)
	{
		MEM_TRACE(__FUNCTION__ ": dest is null.");
		return false;
	}

	if (!size)
	{
		MEM_TRACE(__FUNCTION__ ": size is %d.", size);
		return false;
	}

	if (!VirtualProtect(dest, size, PAGE_READWRITE, &oldProtect))
	{
		MEM_TRACE(__FUNCTION__ ": VirtualProtect returned %d at stage 1", GetLastError());
		return false;
	}

	memcpy(dest, source, size);

	if (!VirtualProtect(dest, size, oldProtect, &oldProtect))
	{
		MEM_TRACE(__FUNCTION__ ": VirtualProtect returned %d at stage 2", GetLastError());
		return false;
	}

	return true;
}

bool WriteRelative(void *address, void *base, void *value, int flags)
{
	return WritePrimitive<void *>(address, Relative(base, value), 0);
}

bool Fill(void *address, uint8_t data, size_t size)
{
	DWORD oldProtect;

	if (!address)
	{
		MEM_TRACE(__FUNCTION__ ": address is null.");
		return false;
	}

	if (!size)
	{
		MEM_TRACE(__FUNCTION__ ": size is %d.", size);
		return false;
	}

	if (!VirtualProtect(address, size, PAGE_READWRITE, &oldProtect))
	{
		MEM_TRACE(__FUNCTION__ ": VirtualProtect returned %d at stage 1", GetLastError());
		return false;
	}

	memset(address, data, size);

	if (!VirtualProtect(address, size, oldProtect, &oldProtect))
	{
		MEM_TRACE(__FUNCTION__ ": VirtualProtect returned %d at stage 2", GetLastError());
		return false;
	}

	return true;
}

#pragma endregion

#pragma region Memory Search

void *FindPattern(void *start, void *left, void *right, void *value, size_t size, int flags)
{
	bool back, ignoreFF, ignore00;
	uint8_t *ret;

	if (!start)
	{
		MEM_TRACE(__FUNCTION__ ": start is null.");
		return nullptr;
	}

	if (!left)
	{
		MEM_TRACE(__FUNCTION__ ": left is null.");
		return nullptr;
	}

	if (!right)
	{
		MEM_TRACE(__FUNCTION__ ": right is null.");
		return nullptr;
	}

	if (!value)
	{
		MEM_TRACE(__FUNCTION__ ": value is null.");
		return nullptr;
	}

	if (!size)
	{
		MEM_TRACE(__FUNCTION__ ": size is 0.");
		return nullptr;
	}

	back = (flags & kPatternFlagsBack) ? true : false;
	ignoreFF = (flags & kPatternFlagsIgnoreFF) ? true : false;
	ignore00 = (flags & kPatternFlagsIgnore00) ? true : false;
	ret = (uint8_t *)start;

	auto CompareMemory = [](uint8_t *p1, uint8_t *p2, size_t len, bool ignoreFF, bool ignore00) -> bool
	{
		void *end;
		uint8_t b;

		end = Transpose(p2, len);

		while (true)
		{
			if (p2 == end)
				break;

			b = *p2;

			if (ignoreFF && (b == 0xFF))
			{
				p1++;
				p2++;
				continue;
			}

			if (ignore00 && (b == 0x00))
			{
				p1++;
				p2++;
				continue;
			}

			if (*p1 != *p2)
				return false;

			p1++;
			p2++;
		}

		return true;
	};

	while (true)
	{
		if (!Bounds(ret, left, right))
			return nullptr;

		if (*(uint8_t *)ret == ((uint8_t *)value)[0])
		{
			if (CompareMemory(ret, (uint8_t *)value, size, ignoreFF, ignore00))
				return ret;
		}

		if (back)
			ret--;
		else
			ret++;
	}

	return nullptr;
}

void *FindReference(void *start, void *left, void *right, void *refaddr, uint16_t opcode, bool back)
{
	const int SizeOfPtr = sizeof(void *);
	int flags;
	bool isTwoBytes, isOpcode;
	void *f;
	uint8_t *ret;

	if (!start)
	{
		MEM_TRACE(__FUNCTION__ ": start is null.");
		return nullptr;
	}

	if (!left)
	{
		MEM_TRACE(__FUNCTION__ ": left is null.");
		return nullptr;
	}

	if (!right)
	{
		MEM_TRACE(__FUNCTION__ ": right is null.");
		return nullptr;
	}

	isOpcode = opcode != 0;
	isTwoBytes = isOpcode && opcode > 255;
	flags = back ? (kPatternFlagsBack | kPatternFlagsUnsafe) : (kPatternFlagsUnsafe);

	right = Transpose(right, -SizeOfPtr);
	ret = (uint8_t *)start;

	auto IsAbsoluteOpcode = [](uint16_t opcode) -> bool
	{
		return opcode != 0x68;
	};

	while (true)
	{
		if (!Bounds(ret, left, right))
			return nullptr;

		if (isOpcode)
		{
			if (isTwoBytes)
				ret = (uint8_t *)FindPrimitive<uint16_t>(ret, left, right, opcode, flags);
			else
				ret = FindPrimitive<uint8_t>(ret, left, right, (uint8_t)opcode, flags);

			if (!ret)
				return nullptr;

			f = ret;
			f = Transpose(f, isTwoBytes ? 2 : 1);

			if (IsAbsoluteOpcode(opcode))
				f = Absolute(f, 0);
			else
				f = *(void **)f;
		}
		else
		{
			f = *(void **)ret;
		}

		if (!refaddr || refaddr == f)
			return ret;

		if (back)
			ret--;
		else
			ret++;
	}

	return nullptr;
}

void *FindRelative(void *start, void *left, void *right, uint16_t opcode, int index, bool back)
{
	void *ret;

	if (!start)
	{
		MEM_TRACE(__FUNCTION__ ": start is null.");
		return nullptr;
	}

	if (!left)
	{
		MEM_TRACE(__FUNCTION__ ": left is null.");
		return nullptr;
	}

	if (!right)
	{
		MEM_TRACE(__FUNCTION__ ": right is null.");
		return nullptr;
	}

	ret = start;

	while (true)
	{
		ret = FindReference(ret, left, right, nullptr, opcode, back);

		if (!ret)
			return nullptr;

		if (index <= 0)
			return ret;

		index--;
	}
}

#pragma endregion

#pragma region Memory Detour

bool WriteFunc(void *call_from, void *call_to, uint8_t opcode)
{
	if (!call_from)
	{
		MEM_TRACE(__FUNCTION__ ": call_from is null.");
		return false;
	}

	if (!call_to)
	{
		MEM_TRACE(__FUNCTION__ ": call_to is null.");
		return false;
	}

	if (!WritePrimitive<uint8_t>(call_from, opcode, 0))
	{
		MEM_TRACE(__FUNCTION__ ": Could not write opcode.");
		return false;
	}

	call_from = Transpose(call_from, 1);
	if (!WriteRelative(call_from, call_from, call_to, kWriteFlagsNone))
	{
		MEM_TRACE(__FUNCTION__ ": Could not write relative.");
		return false;
	}

	return true;
}

bool WriteCall(void *call_from, void *call_to)
{
	return WriteFunc(call_from, call_to, 0xE8);
}

bool WriteJump(void *call_from, void *call_to)
{
	return WriteFunc(call_from, call_to, 0xE9);
}

void *HookRegular(void *addr, void *func)
{
	int codesize;
	int opsize;
	DetourInfo *detour;

	if (!addr)
	{
		MEM_TRACE(__FUNCTION__ ": addr is null.");
		return nullptr;
	}

	if (!func)
	{
		MEM_TRACE(__FUNCTION__ ": func is null.");
		return nullptr;
	}

	codesize = 0;

	while (codesize < 5)
	{
		opsize = InstructionLength(Transpose(addr, codesize));
		if (!opsize)
		{
			MEM_TRACE(__FUNCTION__ ": Could not determine instruction size.");
			return nullptr;
		}

		codesize += opsize;
	}

	detour = DetourInfo::Create(codesize);
	if (!detour)
	{
		MEM_TRACE(__FUNCTION__ ": Could not allocate detour cell.");
		return nullptr;
	}

	memcpy(detour->GetCode(), addr, codesize);
	WriteJump(detour->GetJump(), Transpose(addr, codesize));
	WriteJump(addr, func);

	return detour->GetCode();
}

void *HookExport(HMODULE mod, const char *funcname, void *funcaddr)
{
	void *p;

	if (!mod)
	{
		MEM_TRACE(__FUNCTION__ ": mod is null.");
		return nullptr;
	}

	if (!funcname || !*funcname)
	{
		MEM_TRACE(__FUNCTION__ ": funcname is not set.");
		return nullptr;
	}

	if (!funcaddr)
	{
		MEM_TRACE(__FUNCTION__ ": funcaddr is null.");
		return nullptr;
	}

	p = GetProcAddress(mod, funcname);
	if (!p)
	{
		MEM_TRACE(__FUNCTION__ ": Could not find export.");
		return nullptr;
	}

	return HookRegular(p, funcaddr);
}

bool RestoreHook(void *addr)
{
	void *original;
	DetourInfo *detour;

	if (!addr)
	{
		MEM_TRACE(__FUNCTION__ ": addr is null.");
		return false;
	}

	detour = (DetourInfo *)Transpose(addr, sizeof(*detour));

	if (!detour->IsValid())
	{
		MEM_TRACE(__FUNCTION__ ": Trying to restore non-detour data.");
		return false;
	}

	original = Absolute(Transpose(detour->GetJump(), 1), 0);
	WriteData(Transpose(original, DetourInfo::AsmJumpSize), detour->GetCode(), detour->GetSize(), kWriteFlagsNone);
	detour->Destroy();

	return true;
}

int HookRefAddr(void *original, void *hook, void *start, void *end, uint8_t opcode)
{
	int opsize;
	int ret;
	void *p;

	if (!original)
	{
		MEM_TRACE(__FUNCTION__ ": original is null.");
		return false;
	}

	if (!start)
	{
		MEM_TRACE(__FUNCTION__ ": start is null.");
		return false;
	}

	if (!start)
	{
		MEM_TRACE(__FUNCTION__ ": start is null.");
		return false;
	}

	if (!end)
	{
		MEM_TRACE(__FUNCTION__ ": end is null.");
		return false;
	}

	opsize = (opcode > 0) ? 1 : 0;
	ret = 0;
	p = start;
	end = Transpose(end, -5);

	if (opcode != 0)
	{
		while (true)
		{
			p = FindRelative(p, start, end, opcode, 0, false);
			if (!p)
				return ret;

			if (Absolute(Transpose(p, 1), 0) == original)
			{
				WriteFunc(p, hook, opcode);
				ret++;
			}

			p = Transpose(p, opsize + sizeof(original));
		}
	}
	else
	{
		while (true)
		{
			p = FindPrimitive<void *>(p, start, end, original, kPatternFlagsNone);
			if (!p)
				return ret;

			WritePrimitive<void *>(p, hook, 0);
			p = Transpose(p, sizeof(original));
			ret++;
		}
	}

	return ret;
}

int HookRefCall(void *original, void *hook, void *start, void *end)
{
	return HookRefAddr(original, hook, start, end, 0xE8);
}

int HookRefJump(void *original, void *hook, void *start, void *end)
{
	return HookRefAddr(original, hook, start, end, 0xE9);
}

#pragma endregion

#pragma region Memory Section

IMAGE_SECTION_HEADER *GetSectionByFlags(void *base, long flags, bool pedantic)
{
	auto dos = (IMAGE_DOS_HEADER *)base;
	auto nt = (IMAGE_NT_HEADERS *)(Transpose(dos, dos->e_lfanew));
	auto sec = (IMAGE_SECTION_HEADER *)Transpose(&nt->OptionalHeader, nt->FileHeader.SizeOfOptionalHeader);

	if (pedantic)
	{
		for (int i = 0; i < nt->FileHeader.NumberOfSections; i++)
		{
			if (sec->Characteristics == flags)
				return sec;

			sec++;
		}
	}
	else
	{
		for (int i = 0; i < nt->FileHeader.NumberOfSections; i++)
		{
			if ((sec->Characteristics & flags) != 0)
				return sec;

			sec++;
		}
	}

	return nullptr;
}

IMAGE_SECTION_HEADER *GetSectionByName(void *base, const char *name)
{
	auto dos = (IMAGE_DOS_HEADER *)base;
	auto nt = (IMAGE_NT_HEADERS *)(Transpose(dos, dos->e_lfanew));
	auto sec = (IMAGE_SECTION_HEADER *)Transpose(&nt->OptionalHeader, nt->FileHeader.SizeOfOptionalHeader);

	for (int i = 0; i < nt->FileHeader.NumberOfSections; i++)
	{
		if (!_stricmp((const char *)sec->Name, name))
			return sec;

		sec++;
	}

	return nullptr;
}

IMAGE_SECTION_HEADER *GetRDataSection(void *base)
{
	return GetSectionByFlags(base, IMAGE_SCN_CNT_INITIALIZED_DATA | IMAGE_SCN_MEM_READ);
}

IMAGE_SECTION_HEADER *GetMainCodeSection(void *base)
{
	return GetSectionByFlags(base, IMAGE_SCN_CNT_CODE);
}

bool GetSectionBounds(void *base, IMAGE_SECTION_HEADER *section, void *&secStart, void *&secEnd)
{
	if (!section)
	{
		secStart = nullptr;
		secEnd = nullptr;

		return false;
	}

	secStart = Transpose(base, section->VirtualAddress);
	secEnd = Transpose(secStart, section->Misc.VirtualSize - 1);

	return true;
}

bool GetSectionBounds(IMAGE_SECTION_HEADER *section, void *&secStart, void *&secEnd)
{
	HMODULE base;

	if (!GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
		(LPCSTR)section, &base))
	{
		secStart = nullptr;
		secEnd = nullptr;

		return false;
	}

	return GetSectionBounds((void *)base, section, secStart, secEnd);
}

#pragma endregion

#pragma region Visual C++ RTTI

RTTITypeDescriptor *GetRTTIDescriptor(void *start, void *end, const char *name)
{
	bool bFull;
	char szName[256];
	int namelen;
	RTTITypeDescriptor *td;

	if (!start || !end || !name || !*name)
		return nullptr;

	strcpy(szName, name);

	if (*(uint32_t *)szName == 'VA?.')
	{
		bFull = true;
		namelen = 0;
	}
	else
	{
		bFull = false;
		namelen = strlen(szName);

		*(uint16_t *)&szName[namelen++] = '\0@'; // NOTE(kohtep): Unsafe lifehacks
	}

	void *addr = start;

	while (true)
	{
		addr = FindPrimitive<uint32_t>(addr, start, end, 'VA?.', 0);
		if (!addr)
			return nullptr;

		addr = Transpose(addr, 4);
		td = (RTTITypeDescriptor *)((int)addr - offsetof(RTTITypeDescriptor, name) - 4);

		if (bFull)
		{
			if (!_stricmp(td->name, szName))
				return td;
		}
		else
		{
			if (!_strnicmp((char *)(td->name + 4), szName, namelen))
				return td;
		}
	}
}

void **GetVTableForDescriptor(void *start, void *end, RTTITypeDescriptor *desc)
{
	void *addr;
	RTTICompleteObjectLocator *p;

	if (!start || !end || !desc)
		return nullptr;

	addr = start;
	while (true)
	{
		addr = FindReference(addr, start, end, desc, 0x00, false);
		if (!addr)
			return nullptr;

		p = (RTTICompleteObjectLocator *)((uintptr_t)addr - offsetof(RTTICompleteObjectLocator, pTypeDescriptor));
		if (!p->signature && !p->offset && !p->cdOffset)
		{
			addr = FindReference(start, start, end, p, 0x00, false);

			if (addr)
				return (void **)Transpose(addr, sizeof(void *));

			return nullptr;
		}

		addr = Transpose(addr, 1);
	}

	return nullptr;
}

void **GetVTableForClass(void *start, void *end, const char *name)
{
	RTTITypeDescriptor *desc;

	desc = GetRTTIDescriptor(start, end, name);
	if (!desc)
		return nullptr;

	return GetVTableForDescriptor(start, end, desc);
}

#pragma endregion

DetourInfo *DetourInfo::Create(uint32_t codeSize)
{
	DetourInfo *detour;

	detour = (DetourInfo *)GetExecMem(sizeof(*detour) + codeSize + AsmJumpSize);

	*(uint32_t *)&detour->magic = DetourMagicNumber;
	detour->codeSize = codeSize;

	return detour;
}

void DetourInfo::Destroy()
{
	FreeMemory(this);
}

bool DetourInfo::IsValid()
{
	return *(uint32_t *)&magic == DetourMagicNumber;
}

void *DetourInfo::GetCode()
{
	return (void *)((int)this + sizeof(*this));
}

void *DetourInfo::GetJump()
{
	return (void *)((int)GetCode() + codeSize);
}

uint32_t DetourInfo::GetSize()
{
	return sizeof(*this) + codeSize;
}
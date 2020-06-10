#pragma once

#include <Windows.h>
#include <stdint.h>

#define MEM_TRACE(fmt, ...)

enum PatternFlags
{
	kPatternFlagsNone = 0,
	kPatternFlagsBack = 1 << 0,
	kPatternFlagsIgnoreFF = 1 << 1,
	kPatternFlagsIgnore00 = 1 << 2,
	kPatternFlagsUnsafe = 1 << 3,

	// FindXXString flags

	kPatternFlagsStringRef = 1 << 4,
	kPatternFlagsStringPartial = 1 << 5,
	kPatternFlagsStringDeep = 1 << 6,
};

enum WriteFlags
{
	kWriteFlagsNone = 0,
	WriteFlagsNoProtect = 1 << 0,
};

struct DetourInfo
{
public:
	static const int AsmJumpSize = 5;
	static const uint32_t DetourMagicNumber = 0x01010101;

public:
	uint8_t magic[4];
	uint32_t codeSize;

	DetourInfo() = default;

	static DetourInfo *Create(uint32_t codeSize);

	void Destroy();
	bool IsValid();
	void *GetCode();
	void *GetJump();
	uint32_t GetSize();
};

#pragma region Memory Management

bool FreeMemory(void *data);
void *GetMem(size_t size);
void *GetExecMem(size_t size);

bool ValidateMemory(void *addr);
bool IsExecutable(void *addr);

void *GetBaseAddr(void *addr);
unsigned int GetModuleSize(void *base);

#pragma endregion

#pragma region Memory Basic

void *Transpose(void *addr, int offset);
void *Absolute(void *address, int offset);
void *Relative(void *to, void *address);

template<typename T>
bool Check(void *address, T value, int offset = 0)
{
	if (!address)
	{
		MEM_TRACE(__FUNCTION__ ": Invalid address.");
		return false;
	}

	return *(T *)&((unsigned char *)address)[offset] == value;
}

bool Bounds(void *address, void *bottom, void *top);

#pragma endregion

#pragma region Memory Read

template<typename T> T
ReadPrimitive(void *address, int offset)
{
	if (address == nullptr)
		MEM_TRACE(__FUNCTION__ ": Invalid address.");

	address = (void *)((int)address + offset);

	return *(T *)address;
}

bool ReadData(void *dest, void *source, size_t count);

#pragma endregion

#pragma region Memory Write

bool WriteData(void *dest, void *source, size_t size, int flags);

template<typename T>
bool WritePrimitive(void *address, T value, int offset = 0)
{
	address = Transpose(address, offset);
	return WriteData(address, &value, sizeof(T), kWriteFlagsNone);
}

bool WriteRelative(void *address, void *base, void *value, int flags);

bool Fill(void *address, uint8_t data, size_t size);

#pragma endregion

#pragma region Memory Search

template<typename T>
T *FindPrimitive(void *start, void *left, void *right, T value, int flags)
{
	const int SizeOfT = sizeof(T);
	bool back;
	uint8_t *ret;

	if (!start)
	{
		MEM_TRACE(__FUNCTION__ ": start is null.");
		return nullptr;
	}

	if (!start)
	{
		MEM_TRACE(__FUNCTION__ ": start is null.");
		return nullptr;
	}

	right = (flags & kPatternFlagsUnsafe) ? right : Transpose(right, -SizeOfT);
	back = (flags & kPatternFlagsBack) ? true : false;
	ret = (uint8_t *)start;

	while (true)
	{
		if (!Bounds(ret, left, right))
			return nullptr;

		switch (sizeof(T))
		{
		case 1: case 2: case 4: case 8: case 10:
		{
			if (*(T *)ret == *(T *)&value)
				return (T *)ret;
		}

		default:
		{
			if (!memcmp(ret, &value, sizeof(T)))
				return (T *)ret;
		}
		}

		if (back)
			ret--;
		else
			ret++;
	}

	return nullptr;
}

void *FindPattern(void *start, void *left, void *right, void *value, size_t size, int flags);
void *FindReference(void *start, void *left, void *right, void *refaddr, uint16_t opcode, bool back);
void *FindRelative(void *start, void *left, void *right, uint16_t opcode, int index, bool back);

#pragma endregion

#pragma region Memory Detour

bool WriteFunc(void *call_from, void *call_to, uint8_t opcode);
bool WriteCall(void *call_from, void *call_to);
bool WriteJump(void *call_from, void *call_to);

void *HookRegular(void *addr, void *func);
template <typename T> T HookRegular(void *addr, T func)
{
	return (T)::HookRegular(addr, (void *)func);
}

void *HookExport(HMODULE mod, const char *funcname, void *funcaddr);

bool RestoreHook(void *addr);

int HookRefAddr(void *original, void *hook, void *start, void *end, uint8_t opcode);
int HookRefCall(void *original, void *hook, void *start, void *end);
int HookRefJump(void *original, void *hook, void *start, void *end);

#pragma endregion

#pragma region Memory Section

IMAGE_SECTION_HEADER *GetSectionByFlags(void *base, long flags, bool pedantic = true);
IMAGE_SECTION_HEADER *GetSectionByName(void *base, const char *name);
IMAGE_SECTION_HEADER *GetRDataSection(void *base);
IMAGE_SECTION_HEADER *GetMainCodeSection(void *base);
bool GetSectionBounds(void *base, IMAGE_SECTION_HEADER *section, void *&secStart, void *&secEnd);
bool GetSectionBounds(IMAGE_SECTION_HEADER *section, void *&secStart, void *&secEnd);

#pragma endregion

#pragma region Visual C++ RTTI

struct RTTITypeDescriptor
{
	void **pVFTable;
	void *spare;
	char name[1];
};

struct PMD
{
	int mdisp;
	int pdisp;
	int vdisp;
};

struct RTTIBaseClassDescriptor
{
	RTTITypeDescriptor *pTypeDescriptor;
	unsigned long numContainedBases;
	PMD where;
	unsigned long attributes;
};

struct RTTIBaseClassArray
{
	RTTIBaseClassDescriptor *arrayOfBaseClassDescriptors[1];
};

struct RTTIClassHierarchyDescriptor
{
	unsigned long signature;
	unsigned long offset;
	unsigned long numBaseClasses;
	RTTIBaseClassArray *pBaseClassArray;
};

struct RTTICompleteObjectLocator
{
	unsigned long signature;
	unsigned long offset;
	unsigned long cdOffset;
	RTTITypeDescriptor *pTypeDescriptor;
	RTTIClassHierarchyDescriptor *pClassDescriptor;
};

RTTITypeDescriptor *GetRTTIDescriptor(void *start, void *end, const char *name);
void **GetVTableForDescriptor(void *start, void *end, RTTITypeDescriptor *desc);
void **GetVTableForClass(void *start, void *end, const char *name);

#pragma endregion
#pragma once

#include <iostream>
#include <Windows.h>
#include <Utils\Memory.h>

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

static RTTITypeDescriptor *GetRTTIDescriptor(TModule &rModule, const char *pszName)
{
	if (!pszName || !*pszName)
		return nullptr;

	bool bFull;
	char szName[256];
	int namelen;

	strcpy(szName, pszName);

	if (*(int *)szName == 'VA?.')
	{
		bFull = true;
		namelen = 0;
	}
	else
	{
		bFull = false;
		namelen = strlen(szName);

		*(short *)&szName[namelen++] = '\0@'; // some crappy lifehacks
	}

	auto addr = rModule.pStart;
	auto pEnd = rModule.pEnd;

	while (true)
	{
		addr = Memory::FindLongPtr(addr, pEnd, 'VA?.', 4, false);
		if (!addr)
			return nullptr;

		auto td = (RTTITypeDescriptor *)((int)addr - offsetof(RTTITypeDescriptor, name) - 4);

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

static void **GetVTableForClass(TModule &rModule, const char *name)
{
	auto td = GetRTTIDescriptor(rModule, name);
	if (!td)
		return nullptr;

	auto addr = rModule.pStart;
	while (true)
	{
		addr = Memory::FindRefAddr(addr, rModule.pEnd, td, 0x00);
		if (!addr)
			return nullptr;

		auto p = (RTTICompleteObjectLocator *)((uintptr_t)addr - offsetof(RTTICompleteObjectLocator, pTypeDescriptor));
		if (!p->signature && !p->offset && !p->cdOffset)
		{
			addr = Memory::FindRefAddr(rModule, p);

			if (addr)
				return (void **)Memory::Transpose(addr, sizeof(void *));

			return nullptr;
		}

		addr = Memory::Transpose(addr, 1);
	}

	return nullptr;
}
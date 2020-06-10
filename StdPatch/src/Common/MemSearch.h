#pragma once

#include <Windows.h>
#include <vector>

#include "MemTools.h"

struct IModule;
struct ISearchPattern;

struct SegmentInfo
{
	char *name;
	void *base;
	void *lastByte;
	unsigned int size;
};

struct IInterface
{
	virtual ~IInterface() {};
};

struct IModule : public IInterface
{
	virtual bool GetLoaded() abstract;
	virtual char *GetName() abstract;
	virtual void *GetBase() abstract;
	virtual void *GetLastByte() abstract;

	virtual SegmentInfo *GetSegmentCode() abstract;
	virtual SegmentInfo *GetSegmentData() abstract;
	virtual SegmentInfo *GetSegmentReadOnly() abstract;

	virtual ISearchPattern *CreatePattern(void *&output, const char *name = nullptr) abstract;

	virtual int HookRefAddr(void *addr, void *newAddr, uint8_t opcode, bool quick = true) abstract;
	virtual int HookRefCall(void *addr, void *newAddr, bool quick = true) abstract;
	virtual int HookRefJump(void *addr, void *newAddr, bool quick = true) abstract;

	virtual void *HookExport(const char *funcName, void *newAddr) abstract;

	virtual void *Transpose(int offset) abstract;

	template <typename T>
	ISearchPattern *CreatePattern(T *&output, const char *name = nullptr)
	{
		return CreatePattern(*(void **)&output, name);
	}
};

class CModule : public IModule
{
private:
	char *m_Name;
	HMODULE m_Base;
	size_t m_Size;
	void *m_End;

	SegmentInfo m_SegmentCode;
	SegmentInfo m_SegmentData;
	SegmentInfo m_SegmentRO;

	std::vector<ISearchPattern *> m_Patterns;

	void Initialize(char *name);

public:
	CModule();
	CModule(char *name);
	CModule(const char *name);
	CModule(HMODULE handle);
	CModule(void *imagebase);

	virtual ~CModule();

	virtual bool GetLoaded();
	virtual char *GetName();
	virtual void *GetBase();
	virtual void *GetLastByte();

	virtual SegmentInfo *GetSegmentCode();
	virtual SegmentInfo *GetSegmentData();
	virtual SegmentInfo *GetSegmentReadOnly();

	virtual ISearchPattern *CreatePattern(void *&output, const char *name = nullptr);

	template <typename T>
	ISearchPattern *CreatePattern(T *&output, const char *name = nullptr)
	{
		return CreatePattern(*(void **)&output, name);
	}

	virtual int HookRefAddr(void *addr, void *newAddr, uint8_t opcode, bool quick);
	virtual int HookRefCall(void *addr, void *newAddr, bool quick);
	virtual int HookRefJump(void *addr, void *newAddr, bool quick);

	virtual void *HookExport(const char *funcName, void *newAddr);

	virtual void *Transpose(int offset);
};

struct ISearchPattern : public IInterface
{
	virtual void FindUInt8(uint8_t value, int flags = kPatternFlagsNone) abstract;
	virtual void FindUInt16(uint16_t value, int flags = kPatternFlagsNone) abstract;
	virtual void FindUInt32(uint32_t value, int flags = kPatternFlagsNone) abstract;

	virtual void FindPattern(void *value, size_t size, int flags = kPatternFlagsNone) abstract;
	virtual void FindAnsiString(const char *value, int flags = kPatternFlagsNone) abstract;
	virtual void FindWideString(const uint16_t *value, int flags = kPatternFlagsNone) abstract;

	virtual void FindRelative(uint16_t opcode, int index = 0, bool back = false) abstract;
	virtual void FindReference(void *refAddr, bool back = false) abstract;
	virtual void FindCall(int index = 0, bool deref = false, bool back = false) abstract;
	virtual void FindJump(int index = 0, bool defer = false, bool back = false) abstract;

	virtual void FindVTable(const char *name) abstract;

	virtual bool CheckUInt8(uint8_t value, int offset) abstract;
	virtual bool CheckUInt16(uint16_t value, int offset) abstract;
	virtual bool CheckUInt32(uint32_t value, int offset) abstract;

	virtual void GetProcedure(const char *name) abstract;
	virtual void GetInterface(const char *name) abstract;

	virtual void Dereference() abstract;
	virtual void Absolute() abstract;
	virtual void Transpose(int value) abstract;
	virtual void Align() abstract;

	virtual void ForceOutput(void *value) abstract;
	virtual void *CurrentOutput() abstract;
};

class CSearchPattern : public ISearchPattern
{
private:
	const char *m_Name;
	CModule *m_Module;
	void **m_Output;

private:
	template <typename T>
	void FindPrimitive(T value, int flags)
	{
		if (!m_Output)
			return;

		*m_Output = ::FindPrimitive<T>(*m_Output, m_Module->GetBase(), m_Module->GetLastByte(), value, flags);
	}

	template <typename T>
	bool Check(T value, int offset)
	{
		if (!m_Output)
			return false;

		return ::Check<T>(*m_Output, value, offset);
	}

public:
	CSearchPattern(CModule *mod, void *&output, const char *name = nullptr);

	virtual ~CSearchPattern()
	{
		// TODO: Debug notification if *m_Output is nullptr
	}

	virtual void FindUInt8(uint8_t value, int flags);
	virtual void FindUInt16(uint16_t value, int flags);
	virtual void FindUInt32(uint32_t value, int flags);

	virtual void FindPattern(void *value, size_t size, int flags);
	virtual void FindAnsiString(const char *value, int flags);
	virtual void FindWideString(const uint16_t *value, int flags);

	virtual void FindRelative(uint16_t opcode, int index = 0, bool back = false);
	virtual void FindReference(void *refAddr, bool back = false);
	virtual void FindCall(int index = 0, bool deref = false, bool back = false);
	virtual void FindJump(int index = 0, bool defer = false, bool back = false);

	virtual void FindVTable(const char *name);

	virtual bool CheckUInt8(uint8_t value, int offset);
	virtual bool CheckUInt16(uint16_t value, int offset);
	virtual bool CheckUInt32(uint32_t value, int offset);

	virtual void GetProcedure(const char *name);
	virtual void GetInterface(const char *name);

	virtual void Dereference();
	virtual void Absolute();
	virtual void Transpose(int value);
	virtual void Align();

	virtual void ForceOutput(void *value);
	virtual void *CurrentOutput();
};
#include <malloc.h>
#include <Windows.h>

#include "MemTools.h"
#include "MemSearch.h"

void CModule::Initialize(char *name)
{
	IMAGE_SECTION_HEADER *code, *data, *rodata;

	m_Patterns = std::vector<ISearchPattern *>();

	if (name && *name)
	{
		m_Name = _strdup(name);
		m_Base = GetModuleHandleA(m_Name);
	}
	else
	{
		m_Name = nullptr;
		m_Base = GetModuleHandleA(NULL);
	}

	if (m_Base != 0)
	{
		m_Size = GetModuleSize(m_Base);
		m_End = ::Transpose(m_Base, m_Size - 1);

		code = GetSectionByFlags(m_Base, IMAGE_SCN_CNT_CODE, false);
		data = GetSectionByFlags(m_Base, IMAGE_SCN_CNT_INITIALIZED_DATA | IMAGE_SCN_MEM_READ | IMAGE_SCN_MEM_WRITE, true);
		rodata = GetSectionByFlags(m_Base, IMAGE_SCN_CNT_INITIALIZED_DATA | IMAGE_SCN_MEM_READ, true);

		if (code)
		{
			m_SegmentCode.name = (char *)code->Name;
			GetSectionBounds(code, m_SegmentCode.base, m_SegmentCode.lastByte);
			m_SegmentCode.size = (size_t)((size_t)m_SegmentCode.lastByte - (size_t)m_SegmentCode.base - 1);
		}

		if (data)
		{
			m_SegmentData.name = (char *)data->Name;
			GetSectionBounds(data, m_SegmentData.base, m_SegmentData.lastByte);
			m_SegmentData.size = (size_t)((size_t)m_SegmentData.lastByte - (size_t)m_SegmentData.base - 1);
		}

		if (rodata)
		{
			m_SegmentRO.name = (char *)rodata->Name;
			GetSectionBounds(rodata, m_SegmentRO.base, m_SegmentRO.lastByte);
			m_SegmentRO.size = (size_t)((size_t)m_SegmentRO.lastByte - (size_t)m_SegmentRO.base - 1);
		}
	}
	else
	{
		m_Size = 0;
		m_End = nullptr;
	}
}

CModule::CModule()
{
	Initialize(nullptr);
}

CModule::CModule(char *name)
{
	Initialize(name);
}

CModule::CModule(const char *name)
{
	Initialize((char *)name);
}

CModule::CModule(HMODULE handle)
{
	char modname[MAX_PATH];
	char *name;

	GetModuleFileNameA(handle, modname, sizeof(modname));

	name = strrchr(modname, '\\');
	if (name)
		name++;
	else
		name = modname;

	Initialize(name);
}

CModule::CModule(void *imagebase)
{
	char modname[MAX_PATH];
	char *name;

	GetModuleFileNameA((HMODULE)imagebase, modname, sizeof(modname));

	name = strrchr(modname, '\\');
	if (name)
		name++;
	else
		name = modname;

	Initialize(name);
}

CModule::~CModule()
{
	free(m_Name);

	for (auto &&iface : m_Patterns)
	{
		delete iface;
	}
}

bool CModule::GetLoaded()
{
	return m_Base != nullptr;
}

char *CModule::GetName()
{
	return m_Name;
}

void *CModule::GetBase()
{
	return m_Base;
}

void *CModule::GetLastByte()
{
	return m_End;
}

SegmentInfo *CModule::GetSegmentCode()
{
	return &m_SegmentCode;
}

SegmentInfo *CModule::GetSegmentData()
{
	return &m_SegmentData;
}

SegmentInfo *CModule::GetSegmentReadOnly()
{
	return &m_SegmentRO;
}

ISearchPattern *CModule::CreatePattern(void *&output, const char *name)
{
	ISearchPattern *pattern;

	pattern = reinterpret_cast<ISearchPattern *>(new CSearchPattern(this, output, name));
	m_Patterns.push_back(pattern);

	return pattern;
}

int CModule::HookRefAddr(void *addr, void *newAddr, uint8_t opcode, bool quick)
{
	void *start, *end;

	if (quick)
	{
		start = m_SegmentCode.base;
		end = m_SegmentCode.lastByte;
	}
	else
	{
		start = m_Base;
		end = m_End;
	}

	return ::HookRefAddr(addr, newAddr, start, end, opcode);
}

int CModule::HookRefCall(void *addr, void *newAddr, bool quick)
{
	void *start, *end;

	if (quick)
	{
		start = m_SegmentCode.base;
		end = m_SegmentCode.lastByte;
	}
	else
	{
		start = m_Base;
		end = m_End;
	}

	return ::HookRefCall(addr, newAddr, start, end);
}

int CModule::HookRefJump(void *addr, void *newAddr, bool quick)
{
	void *start, *end;

	if (quick)
	{
		start = m_SegmentCode.base;
		end = m_SegmentCode.lastByte;
	}
	else
	{
		start = m_Base;
		end = m_End;
	}

	return ::HookRefJump(addr, newAddr, start, end);
}

void *CModule::HookExport(const char *funcName, void *newAddr)
{
	if (!funcName || !*funcName || !newAddr)
		return nullptr;

	return ::HookExport(m_Base, funcName, newAddr);
}

void *CModule::Transpose(int offset)
{
	if (m_Base == nullptr)
		return nullptr;

	return ::Transpose(m_Base, offset);
}

CSearchPattern::CSearchPattern(CModule *mod, void *&output, const char *name)
{
	m_Name = name;
	m_Module = mod;
	m_Output = (void **)&output;
	*m_Output = m_Module->GetBase();
}

void CSearchPattern::FindUInt8(uint8_t value, int flags)
{
	if (!*m_Output)
		return;

	FindPrimitive(value, flags);
}

void CSearchPattern::FindUInt16(uint16_t value, int flags)
{
	if (!*m_Output)
		return;

	FindPrimitive(value, flags);
}

void CSearchPattern::FindUInt32(uint32_t value, int flags)
{
	if (!*m_Output)
		return;

	FindPrimitive(value, flags);
}

void CSearchPattern::FindPattern(void *value, size_t size, int flags)
{
	if (!*m_Output)
		return;

	*m_Output = ::FindPattern(*m_Output, m_Module->GetBase(), m_Module->GetLastByte(), value, size, flags);
}

void CSearchPattern::FindAnsiString(const char *value, int flags)
{
	int len;
	uint8_t *pattern;
	SegmentInfo *seginfo;

	if (!*m_Output)
		return;

	if (!value || !*value)
	{
		*m_Output = nullptr;
		return;
	}

	len = strlen(value);

	if (!(flags & kPatternFlagsStringPartial))
		len += sizeof(value[0]);

	pattern = (uint8_t *)_malloca(len);
	memcpy(pattern, value, len);

	if (flags & kPatternFlagsStringDeep)
	{
		*m_Output = ::FindPattern(m_Module->GetBase(), m_Module->GetBase(), m_Module->GetLastByte(), pattern, len, kPatternFlagsNone);
	}
	else
	{
		seginfo = m_Module->GetSegmentReadOnly();
		*m_Output = ::FindPattern(seginfo->base, seginfo->base, seginfo->lastByte, pattern, len, kPatternFlagsNone);
	}

	if (*m_Output == nullptr)
		return;

	if (flags & kPatternFlagsStringRef)
	{
		if (flags & kPatternFlagsStringDeep)
		{
			*m_Output = ::FindReference(m_Module->GetBase(), m_Module->GetBase(), m_Module->GetLastByte(), *m_Output, kPatternFlagsNone, false);
		}
		else
		{
			seginfo = m_Module->GetSegmentCode();
			*m_Output = ::FindReference(seginfo->base, seginfo->base, seginfo->lastByte, *m_Output, kPatternFlagsNone, false);
		}
	}
}

void CSearchPattern::FindWideString(const uint16_t *value, int flags)
{
	// TODO: Implement
}

void CSearchPattern::FindRelative(uint16_t opcode, int index, bool back)
{
	if (!*m_Output)
		return;

	*m_Output = ::FindRelative(*m_Output, m_Module->GetSegmentCode()->base, m_Module->GetSegmentCode()->lastByte,
		opcode, index, back);
}

void CSearchPattern::FindReference(void *refAddr, bool back)
{
	if (!*m_Output)
		return;

	*m_Output = ::FindReference(*m_Output, m_Module->GetBase(), m_Module->GetLastByte(), refAddr, 0, false);
}

void CSearchPattern::FindCall(int index, bool deref, bool back)
{
	if (!*m_Output)
		return;

	*m_Output = ::FindRelative(*m_Output, m_Module->GetSegmentCode()->base, m_Module->GetSegmentCode()->lastByte,
		0xE8, index, back);

	if (*m_Output && deref)
	{
		*m_Output = ::Transpose(*m_Output, 1);
		*m_Output = ::Absolute(*m_Output, 0);
	}
}

void CSearchPattern::FindJump(int index, bool defer, bool back)
{
	if (!*m_Output)
		return;

	*m_Output = ::FindRelative(*m_Output, m_Module->GetSegmentCode()->base, m_Module->GetSegmentCode()->lastByte,
		0xE9, index, back);

	if (*m_Output && defer)
	{
		*m_Output = ::Transpose(*m_Output, 1);
		*m_Output = ::Absolute(*m_Output, 0);
	}
}

void CSearchPattern::FindVTable(const char *name)
{
	RTTITypeDescriptor *desc;

	desc = GetRTTIDescriptor(m_Module->GetSegmentData()->base, m_Module->GetSegmentData()->lastByte, name);
	*m_Output = GetVTableForDescriptor(m_Module->GetSegmentReadOnly()->base, m_Module->GetSegmentReadOnly()->lastByte, desc);
}

bool CSearchPattern::CheckUInt8(uint8_t value, int offset)
{
	if (!*m_Output)
		return false;

	return Check<uint8_t>(value, offset);
}

bool CSearchPattern::CheckUInt16(uint16_t value, int offset)
{
	if (!*m_Output)
		return false;

	return Check<uint16_t>(value, offset);
}

bool CSearchPattern::CheckUInt32(uint32_t value, int offset)
{
	if (!*m_Output)
		return false;

	return Check<uint32_t>(value, offset);
}

void CSearchPattern::GetProcedure(const char *name)
{
	*m_Output = GetProcAddress((HMODULE)m_Module->GetBase(), name);
}

void CSearchPattern::GetInterface(const char *name)
{
	void*(__cdecl *f)(const char *name, int *retcode);

	f = decltype(f)(GetProcAddress((HMODULE)m_Module->GetBase(), "CreateInterface"));

	if (!f)
	{
		*m_Output = nullptr;
		return;
	}

	*m_Output = (void **)f(name, nullptr);
}

void CSearchPattern::Dereference()
{
	if (*m_Output)
		*m_Output = *(void **)*m_Output;
}

void CSearchPattern::Absolute()
{
	if (*m_Output)
		*m_Output = ::Absolute(*m_Output, 0);
}

void CSearchPattern::Transpose(int value)
{
	if (*m_Output)
		*m_Output = ::Transpose(*m_Output, value);
}

void CSearchPattern::Align()
{
	if (*m_Output)
		*m_Output = (void *)((size_t)*m_Output & ~0xF);
}

void CSearchPattern::ForceOutput(void *value)
{
	if (!value || !::Bounds(value, m_Module->GetBase(), m_Module->GetLastByte()))
	{
		*m_Output = nullptr;
		return;
	}

	*m_Output = value;
}

void *CSearchPattern::CurrentOutput()
{
	return *m_Output;
}

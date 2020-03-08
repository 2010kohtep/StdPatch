#pragma once

#include <vector>

struct IPatch
{
	virtual void Enable() abstract;
	virtual void Disable() abstract;
};

enum DetourMethod
{
	kNoDetour,    // Do nothing, also should be marked as invalid value
	kRegularCall, // Write 'call XXX' to address
	kRegularJump, // Write 'jump XXX' to address
	kReferences,  // Replace all references to address
};

struct IDetour
{
	virtual void Enable() abstract;
	virtual void Disable() abstract;
};

struct ISearch
{
private:
	const char *m_pszSearchName;
	void **m_pResult;

	std::vector<IPatch *> m_Patches;
	std::vector<IDetour *> m_Detours;
public:
	ISearch() = delete;

	ISearch(const char *searchName, void **output)
	{
		m_pszSearchName = searchName;
		m_pResult = output;

		m_Patches = std::vector<IPatch *>();
		m_Detours = std::vector<IDetour *>();
	}

	const char *GetName()
	{
		return m_pszSearchName;
	}

	void Init()
	{
		for (auto &&patch : m_Patches)
		{

		}

		for (auto &&detour : m_Detours)
		{

		}
	}

	virtual bool Find() abstract;
};

class CSearchMgr
{
private:
	std::vector<ISearch *> m_Searches;
public:
	CSearchMgr()
	{
		m_Searches = std::vector<ISearch *>();
	}

	void FindAll()
	{
		for (auto &&search : m_Searches)
		{
			if (!search->Find())
			{
				//Print("WARNING! Variable '%s' could not be found.", search->GetName());
			}
		}
	}
};
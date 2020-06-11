#pragma once

#include <vector>

struct IMod;

class CModMgr
{
private:
	std::vector<IMod *> m_Mods;

public:
	CModMgr()
	{
		m_Mods = std::vector<IMod *>();
	}

	void RegisterMod(IMod *mod);
	void InitMods();
};

extern CModMgr gModMgr;

struct IMod
{
	IMod()
	{
		gModMgr.RegisterMod(this);
	}

	virtual ~IMod() {}

	virtual const char *GetName() abstract;
	virtual bool Find() abstract;
	virtual bool Patch() abstract;
};
#pragma once

#include "common/Memory.h"
#include "common/ArrayHelper.h"

#include "main/SDK.h"

extern TModule g_Base;

extern int g_nMAXSTUDIOVERTS_NEW;
extern int g_nBUFFERSIZE_NEW;
extern int g_nMAXFLEXCONTROLLER_NEW;

extern int g_nMAXSTUDIOVERTS_DEF;
extern int g_nBUFFERSIZE_DEF;
extern int g_nMAXFLEXCONTROLLER_DEF;

extern CArrayHelper<TFlexController> g_FlexControllerNew;
extern CArrayHelper<TVUnify *> g_VerticesPtrsNew;
extern CArrayHelper<TVUnify> g_VerticesDataNew;
extern CArrayHelper<TWeightList> g_WeightList;

extern void *g_pBUFFERSIZE;
extern void *g_pMAXSTUDIOVERTS;
extern void *g_pMAXFLEXCONTROLLER;

using TAddToVlist = void *(__cdecl *)(int a1, int a2, int a3, int a4);
using TIsInt24 = bool(__cdecl *)(int nValue);

extern TAddToVlist g_pfnAddToVlist;
extern TIsInt24 g_pfnIsInt24;

extern void *g_pVList;
extern void *g_pFlexController;
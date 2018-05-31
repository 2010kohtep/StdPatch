#include "Global.h"
#include <Utils/Memory.h>

TModule g_Base;

int g_nMAXSTUDIOVERTS_NEW    = 0x00F00000; // def - $80000
int g_nBUFFERSIZE_NEW        = 0x02000000; // def - $2000000
int g_nMAXFLEXCONTROLLER_NEW = 0x00000400; // def - $80

int g_nMAXSTUDIOVERTS_DEF;
int g_nBUFFERSIZE_DEF;
int g_nMAXFLEXCONTROLLER_DEF;

CArrayHelper<TFlexController> g_FlexControllerNew;
CArrayHelper<TVUnify *>       g_VerticesPtrsNew;
CArrayHelper<TVUnify>         g_VerticesDataNew;
CArrayHelper<TWeightList>     g_WeightList;

void *g_pBUFFERSIZE;
void *g_pMAXSTUDIOVERTS;
void *g_pMAXFLEXCONTROLLER;

TAddToVlist g_pfnAddToVlist;
TIsInt24    g_pfnIsInt24;

void *g_pVList;
void *g_pFlexController;
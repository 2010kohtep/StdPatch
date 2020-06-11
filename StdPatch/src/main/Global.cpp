#include "main/Global.h"

IModule *gStudioExe;
IModule *gKernelDll;

int g_nMAXSTUDIOVERTS_NEW    = 0x00F00000; // def - $80000
int g_nBUFFERSIZE_NEW        = 0x02000000; // def - $2000000
int g_nMAXFLEXCONTROLLER_NEW = 0x00000400; // def - $80
int g_nMAXMATERIALSCOUNT_NEW = 0x0000007F; // def - $20

int g_nMAXSTUDIOVERTS_DEF;
int g_nBUFFERSIZE_DEF;
int g_nMAXFLEXCONTROLLER_DEF;
int g_nMAXMATERIALSCOUNT_DEF;

CArrayHelper<TFlexController> g_FlexControllerNew;
CArrayHelper<TVUnify *>       g_VerticesPtrsNew;
CArrayHelper<TVUnify>         g_VerticesDataNew;
CArrayHelper<TWeightList>     g_WeightList;
CArrayHelper<TMaterialInfo>   g_MaterialsList;
CArrayHelper<uint32_t>        g_MaterialsIndex;

void *g_pBUFFERSIZE;
void *g_pMAXSTUDIOVERTS;
void *g_pMAXFLEXCONTROLLER;
void *g_pMAXMATERIALSCOUNT;

TAddToVlist g_pfnAddToVlist;
TIsInt24    g_pfnIsInt24;
TCStudioMDLApp_Create g_pfnCStudioMDLApp_Create;
TOutputDebugStringA g_gateOutputDebugStringA;

void *pfnSanityCheckVertexBoneLODFlags_MdlError;

void *g_pVList;
void *g_pMaterialsList;
void *g_pMaterialsIndex;
void *g_pMaterialsListCheck;
void *g_pFlexController;
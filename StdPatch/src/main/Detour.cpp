#include "common/Console.h"
#include "common/Memory.h"

#include "main/SDK.h"
#include "main/Global.h"

class COptimizedModel;
using TCOptimizedModel_WriteVTXFile = void(__fastcall *)(COptimizedModel *, int, studiohdr_t *, LPCSTR, TotalMeshStats_t *);

TCOptimizedModel_WriteVTXFile orgCOptimizedModel_WriteVTXFile;

void __fastcall COptimizedModel_WriteVTXFile(COptimizedModel *pSelf, int, studiohdr_t *pHdr, const char *pszFileName, TotalMeshStats_t *pStats)
{
	DbgTrace("Begin: COptimizedModel::WriteVTXFile\n");

	DbgTrace("\t\tTotalBodyParts = %d\n", pStats->m_TotalBodyParts);
	DbgTrace("\t\tTotalModels = %d\n", pStats->m_TotalModels);
	DbgTrace("\t\tTotalModelLODs = %d\n", pStats->m_TotalModelLODs);
	DbgTrace("\t\tTotalMeshes = %d\n", pStats->m_TotalMeshes);
	DbgTrace("\t\tTotalStrips = %d\n", pStats->m_TotalStrips);
	DbgTrace("\t\tTotalStripGroups = %d\n", pStats->m_TotalStripGroups);
	DbgTrace("\t\tTotalVerts = %d\n", pStats->m_TotalVerts);
	DbgTrace("\t\tTotalIndices = %d\n", pStats->m_TotalIndices);
	DbgTrace("\t\tTotalTopology = %d\n", pStats->m_TotalTopology);
	DbgTrace("\t\tTotalBoneStateChanges = %d\n", pStats->m_TotalBoneStateChanges);
	DbgTrace("\t\tTotalMaterialReplacements = %d\n", pStats->m_TotalMaterialReplacements);

	orgCOptimizedModel_WriteVTXFile(pSelf, 0, pHdr, pszFileName, pStats);

	DbgTrace("End: COptimizedModel::WriteVTXFile\n");
}

void InsertDebugEvents()
{
	ISearchPattern *pattern;
	void *pAddr;

	//
	// Hook COptimizedModel::WriteVTXFile method.
	//

	const uint8_t abPattern[] = { 0x55, 0x8B, 0xEC, 0x81, 0xEC, 0xC0, 0x00, 0x00, 0x00, 0x89 };
	gStudioExe->CreatePattern(pAddr)->FindPattern((void *)&abPattern, sizeof(abPattern));
	orgCOptimizedModel_WriteVTXFile = HookRegular(pAddr, COptimizedModel_WriteVTXFile);


	//
	// Enable debug output for COptimizedModel::SortBonesWithinVertex method.
	//

	pattern = gStudioExe->CreatePattern(pAddr);
	{
		pattern->FindUInt32(0x2851B60F);
		pattern->FindUInt16(0x006A);
		pattern->Transpose(1);
	}

	WritePrimitive<uint8_t>(pAddr, 0x01);
}

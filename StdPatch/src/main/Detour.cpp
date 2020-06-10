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
	//
	// Hook COptimizedModel::WriteVTXFile method.
	//

	{
		const unsigned char abPattern[] = { 0x55, 0x8B, 0xEC, 0x81, 0xEC, 0xC0, 0x00, 0x00, 0x00, 0x89 };

		if (auto pAddr = Memory::FindPattern(g_Base, abPattern, sizeof(abPattern), 0))
		{
			auto pfnTrampoline = Memory::HookRegular(pAddr, COptimizedModel_WriteVTXFile, 9);
			orgCOptimizedModel_WriteVTXFile = TCOptimizedModel_WriteVTXFile(pfnTrampoline);
		}
	}

	//
	// Enable debug output for COptimizedModel::SortBonesWithinVertex method.
	//

	{
		if (auto pAddr = Memory::FindLongPtr(g_Base, 0x2851B60F, 0, false))
		{
			if (pAddr = Memory::FindWordPtr(pAddr, 64, 0x006A, 1, false))
			{
				Memory::WriteByte(pAddr, 0x01);
			}
		}
	}
}

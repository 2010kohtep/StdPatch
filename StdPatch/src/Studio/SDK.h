#pragma once

const int MAXWEIGHTLISTS = 128;
const int MAXSTUDIONAME = 128;
const int MAXSTUDIOBONES = 128;

const int MAXWEIGHTSPERLIST = MAXSTUDIOBONES;

struct TWeightList
{
	unsigned char dummy[5252];
};

struct TFlexController
{
	char szName[127], szFlexType[127];
	float fMin, fMax;
};

struct TVUnify
{
	int nRefCount, nLastRef, nFirstRef;
	int v, m, n, t;
	TVUnify *pNext;
};

struct Vector
{
	float x;
	float y;
	float z;
};

struct studiohdr2_t
{
	int numsrcbonetransform;
	int srcbonetransformindex;
	int illumpositionattachmentindex;
	float flMaxEyeDeflection;
	int linearboneindex;
	int sznameindex;
	int m_nBoneFlexDriverCount;
	int m_nBoneFlexDriverIndex;
	int reserved[56];
};

struct studiohdr_t
{
	int id;
	int version;
	int checksum;
	char name[64];
	int length;
	Vector eyeposition;
	Vector illumposition;
	Vector hull_min;
	Vector hull_max;
	Vector view_bbmin;
	Vector view_bbmax;
	int flags;
	int numbones;
	int boneindex;
	int numbonecontrollers;
	int bonecontrollerindex;
	int numhitboxsets;
	int hitboxsetindex;
	int numlocalanim;
	int localanimindex;
	int numlocalseq;
	int localseqindex;
	int activitylistversion;
	int eventsindexed;
	int numtextures;
	int textureindex;
	int numcdtextures;
	int cdtextureindex;
	int numskinref;
	int numskinfamilies;
	int skinindex;
	int numbodyparts;
	int bodypartindex;
	int numlocalattachments;
	int localattachmentindex;
	int numlocalnodes;
	int localnodeindex;
	int localnodenameindex;
	int numflexdesc;
	int flexdescindex;
	int numflexcontrollers;
	int flexcontrollerindex;
	int numflexrules;
	int flexruleindex;
	int numikchains;
	int ikchainindex;
	int nummouths;
	int mouthindex;
	int numlocalposeparameters;
	int localposeparamindex;
	int surfacepropindex;
	int keyvalueindex;
	int keyvaluesize;
	int numlocalikautoplaylocks;
	int localikautoplaylockindex;
	float mass;
	int contents;
	int numincludemodels;
	int includemodelindex;
	void *virtualModel;
	int szanimblocknameindex;
	int numanimblocks;
	int animblockindex;
	void *animblockModel;
	int bonetablebynameindex;
	void *pVertexBase;
	void *pIndexBase;
	unsigned char constdirectionallightdot;
	unsigned char rootLOD;
	unsigned char numAllowedRootLODs;
	unsigned char unused[1];
	int unused4;
	int numflexcontrollerui;
	int flexcontrolleruiindex;
	float flVertAnimFixedPointScale;
	int unused3[1];
	int studiohdr2index;
	int unused2[1];
	studiohdr2_t header2;
};

struct TotalMeshStats_t
{
	int m_TotalBodyParts;
	int m_TotalModels;
	int m_TotalModelLODs;
	int m_TotalMeshes;
	int m_TotalStrips;
	int m_TotalStripGroups;
	int m_TotalVerts;
	int m_TotalIndices;
	int m_TotalTopology;
	int m_TotalBoneStateChanges;
	int m_TotalMaterialReplacements;
};
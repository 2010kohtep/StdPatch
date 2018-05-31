#pragma once

void FindModules();

bool Hook_MAXSTUDIOVERTS();
bool Patch_SanityCheckVertexBoneLODFlags();
bool Find_IsInt24();
bool Find_BUFFERSIZE();
bool Find_MAXFLEXCONTROLLER();
bool Find_MAXSTUDIOVERTS();
bool Find_VList();
bool Find_AddrToVlist();
bool Hook_VerticesPtrs();
bool Patch_WriteVTXFile();
bool Patch_MAXFLEXCONTROLLER();
bool Hook_FlexController();
bool Hook_IsInt24();
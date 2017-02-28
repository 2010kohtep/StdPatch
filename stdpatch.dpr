library stdpatch;

{$IF COMPILERVERSION >= 17}
  {$DEFINE INLINE}
{$IFEND}

uses
  SysUtils,
  Windows,
  Memory,
  Studio.SDK,
  Studio.Funcs,
  Studio.Utils,
  Studio.Global,
  Studio.Patch;

procedure LoadConfig;
var
  Buf: array[0..MAX_PATH - 1] of Char;
  P: PChar;
begin
  GetModuleFileName(0, Buf, SizeOf(Buf));
  P := StrRScan(Buf, '\');
  if P <> nil then
    P^ := #0;
       
  StrCopy(P, '\stdpatch.ini');

  MAXSTUDIOVERTS_NEW := GetPrivateProfileInt('Main', 'MaxStudioVerts', $80000, Buf);
  BUFFERSIZE_NEW := GetPrivateProfileInt('Main', 'BufferSize', $2000000, Buf);
end;

procedure FailedToFind(const Name: string); {$IFDEF INLINE} inline; {$ENDIF}
begin
  WriteLn('Failed to find "', Name, '".');
end;

procedure FailedToPatch(const Name: string); {$IFDEF INLINE} inline; {$ENDIF}
begin
  WriteLn('Failed to patch "', Name, '".');
end;

procedure FailedToHook(const Name: string); {$IFDEF INLINE} inline; {$ENDIF}
begin
  WriteLn('Failed to hook "', Name, '".');
end;

procedure PatchStudioMdl;
var
  ExeName: string;
begin
  WriteLn('StudioMdl Patcher 1.2.0 is started.');
  WriteLn('Code by Alexander B. special for RED_EYE.');
  WriteLn;

  FindModules;

  ExeName := ExtractFileName(ParamStr(0));
  WriteLn(ExeName, ' Base: 0x', IntToHex(Base, 8), ' (', Base, ').');
  WriteLn(ExeName, ' Size: 0x', IntToHex(Size, 8), ' (', Size, ').');

  if not Find_AddrToVlist then
    FailedToFind('AddtToVlist()')
  else
  begin
    if not Find_VList then
      FailedToFind('Vlist');
  end;

  if not Find_MAXSTUDIOVERTS then
    FailedToFind('MAXSTUDIOVERTS')
  else
    if not Hook_MAXSTUDIOVERTS then
      FailedToHook('MAXSTUDIOVERTS');

  if not Find_IsInt24 then
    FailedToFind('IsInt24()')
  else
    if not Hook_IsInt24 then
      FailedToHook('IsInt24()');

  if not Find_BUFFERSIZE then
    FailedToFind('BUFFERSIZE')
  else
    if not Patch_WriteVTXFile then
      FailedToPatch('WriteVTXFile()');

  if not Patch_SanityCheckVertexBoneLODFlags then
    FailedToPatch('SanityCheckVertexBoneLODFlags()');

  if not Hook_VerticesPtrs then
    FailedToHook('VerticesPtrs');

  WriteLn;
end;

begin
  LoadConfig;

  PatchStudioMdl;
end.

unit Studio.Patch;

interface

uses
  System.Types, System.SysUtils, Winapi.Windows, Studio.Funcs, Studio.Global, Studio.SDK,
  Studio.Utils, Memory;

procedure FindModules;

function Find_AddrToVlist: Boolean;
function Find_MAXSTUDIOVERTS: Boolean;
function Find_BUFFERSIZE: Boolean;
function Find_VList: Boolean;
function Find_IsInt24: Boolean;

function Patch_SanityCheckVertexBoneLODFlags: Boolean;
function Hook_VerticesPtrs: Boolean;

function Patch_WriteVTXFile: Boolean;
function Hook_AddToVList: Boolean;
function Hook_IsInt24: Boolean;
function Hook_MAXSTUDIOVERTS: Boolean;

implementation

procedure FindModules;
begin
  Base := GetModuleHandle(nil);
  Size := GetModuleSize(Base);
end;

function Hook_MAXSTUDIOVERTS: Boolean;
const
  Pattern: array[0..13] of Byte = ($33, $FF, $68, $FF, $FF, $FF, $FF, $FF, $68, $FF, $FF, $FF, $FF, $89); // dangerous
var
  Addr: Pointer;
begin
  if pMAXSTUDIOVERTS = nil then
    Exit(False);

  Addr := FindPattern(Pointer(Base), Size, @Pattern[0], SizeOf(Pattern), 3);
  if Addr <> nil then
    WriteLong(Addr, MAXSTUDIOVERTS_NEW);

  // Mem_WriteLong(pMAXSTUDIOVERTS, MAXSTUDIOVERTS_NEW);
  Exit(True);
end;

function Patch_SanityCheckVertexBoneLODFlags: Boolean;
var
  Addr: Pointer;
begin
  Addr := FindPushString(Pointer(Base), Size, 'Mismarked Bone flag');

  if Addr = nil then
    Exit(False);

  Addr := FindNextCall(Addr, 1);
  InsertCall(Addr, @MdlError);

  // WriteLn('SanityCheckVertexBoneLODFlags(): Patched.');
  Exit(True);
end;

function Find_IsInt24: Boolean;
var
  Addr: Pointer;
begin
  Addr := FindPushString(Pointer(Base), Size, 'int24 conversion out of range %d');
  if Addr = nil then
    Exit(False);

  Addr := FindWordPtr(Addr, 64, $EC8B, -1, True);
  if Addr = nil then
    Exit(False);

  pIsInt24 := Addr;
  Exit(True);
end;

function Find_BUFFERSIZE: Boolean;
const
  Pattern: array[0..13] of Byte = ($89, $85, $FF, $FF, $FF, $FF, $83, $BD, $FF, $FF, $FF, $FF, $00, $74);
var
  Addr: Pointer;
begin
  Addr := FindPattern(Pointer(Base), Size, @Pattern[0], SizeOf(Pattern), SizeOf(Pattern) + 1);

  if Addr = nil then
    Exit(False);

  if not CheckByte(Addr, $68, 0) then
    Exit(False)
  else
  begin
    Addr := Transpose(Addr, 1);
    pBUFFERSIZE := Addr;
    BUFFERSIZE_DEF := PLongWord(Addr)^;
  end;

  Exit(True);
end;

function Find_MAXSTUDIOVERTS: Boolean;
var
  Addr: Pointer;
begin
  Addr := FindPushString(Pointer(Base), Size, 'Too many unified vertices');
  if Addr = nil then
    Exit(False);

  if not CheckByte(Addr, $81, -12) then
    Exit(False);

  Addr := Pointer(Integer(Addr) - 6);
  pMAXSTUDIOVERTS := Addr;
  MAXSTUDIOVERTS_DEF := PLongWord(Addr)^;
  Exit(True);
end;

function Find_VList: Boolean;
var
  Addr: Pointer;
begin
  Addr := @pfnAddToVlist;

  if Addr = nil then
    Exit(False);

  Addr := FindWordPtr(Addr, 64, $048B, 3);

  if Addr = nil then
    Exit(False);

  pVList := PPointer(Addr)^;

  Exit(True);
end;

function Find_AddrToVlist: Boolean;
var
  Addr: Pointer;
begin
  Addr := FindPushString(Pointer(Base), Size, 'Too many unified vertices'#10);

  if Addr = nil then
    Exit(False);

  Addr := FindWordPtr(Addr, 256, $EC8B, -1, True);
  if Addr <> nil then
    pfnAddToVlist := Addr
  else
    Exit(False);

  Exit(True);
end;

function Hook_VerticesPtrs: Boolean;
begin
  SetLength(VerticesPtrsNew, MAXSTUDIOVERTS_NEW);

  if HookRefAddr(Pointer(Base), Size, pVList, @VerticesPtrsNew[0], True) = 0 then
  begin
    SetLength(VerticesPtrsNew, 0);
    Exit(False);
  end;

  // WriteLn('VerticesPtrs: Hooked (', BytesToStr(MAXSTUDIOVERTS_DEF * 4), ' -> ', BytesToStr(MAXSTUDIOVERTS_NEW * 4), ').');
  Exit(True);
end;

function Patch_WriteVTXFile: Boolean;
const
  Pattern: array[0..13] of Byte = ($89, $85, $FF, $FF, $FF, $FF, $83, $BD, $FF, $FF, $FF, $FF, $00, $74);
var
  Addr: Pointer;
begin
  Addr := FindLongPtr(Transpose(pBUFFERSIZE, SizeOf(Integer)), 64, BUFFERSIZE_DEF);
  if Addr <> nil then
  begin
    WriteLong(pBUFFERSIZE, BUFFERSIZE_NEW);
    WriteLong(Addr, BUFFERSIZE_NEW);

    // WriteLn('WriteVTXFile(): Patched (', BytesToStr(BUFFERSIZE_DEF), ' -> ', BytesToStr(BUFFERSIZE_NEW), ').');
    Exit(True);
  end
  else
    Exit(False);
end;

function Hook_AddToVList: Boolean;
begin
  Exit(HookRefCall(Pointer(Base), Size, @pfnAddToVlist, @AddToVlist) <> 0);
end;

function Hook_IsInt24: Boolean;
var
  I: Integer;
begin
  I := HookRefCall(Pointer(Base), Size, @pIsInt24, @IsInt24);

  Exit(I <> 0);
end;

end.

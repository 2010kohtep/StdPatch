unit Studio.Patch;

interface

uses
  Types, SysUtils, Windows, Studio.Funcs, Studio.Global, Studio.SDK,
  Studio.Utils, Memory;

procedure FindModules;

function Find_AddrToVlist: Boolean;
function Find_MAXSTUDIOVERTS: Boolean;
function Find_BUFFERSIZE: Boolean;
function Find_MAXFLEXCONTROLLER: Boolean;
function Find_VList: Boolean;
function Find_IsInt24: Boolean;

function Patch_SanityCheckVertexBoneLODFlags: Boolean;
function Hook_VerticesPtrs: Boolean;

function Patch_WriteVTXFile: Boolean;
function Patch_MAXFLEXCONTROLLER: Boolean;
function Hook_FlexController: Boolean;
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
  begin
    Result := False;
    Exit;
  end;

  Addr := FindPattern(Pointer(Base), Size, @Pattern[0], SizeOf(Pattern), 3);
  if Addr <> nil then
    WriteLong(Addr, MAXSTUDIOVERTS_NEW);

  Result := True;
end;

function Patch_SanityCheckVertexBoneLODFlags: Boolean;
var
  Addr: Pointer;
begin
  Addr := FindPushString(Pointer(Base), Size, 'Mismarked Bone flag');

  if Addr = nil then
  begin
    Result := False;
    Exit;
  end;

  Addr := FindNextCall(Addr, 1);
  InsertCall(Addr, @MdlError);

  Result := True;
end;

function Find_IsInt24: Boolean;
var
  Addr: Pointer;
begin
  Addr := FindPushString(Pointer(Base), Size, 'int24 conversion out of range %d');
  if Addr = nil then
  begin
    Result := False;
    Exit;
  end;

  Addr := FindWordPtr(Addr, 64, $EC8B, -1, True);
  if Addr = nil then
  begin
    Result := False;
    Exit;
  end;

  pIsInt24 := Addr;
  Result := True;
end;

function Find_BUFFERSIZE: Boolean;
const
  Pattern: array[0..13] of Byte = ($89, $85, $FF, $FF, $FF, $FF, $83, $BD, $FF, $FF, $FF, $FF, $00, $74);
var
  Addr: Pointer;
begin
  Addr := FindPattern(Pointer(Base), Size, @Pattern[0], SizeOf(Pattern), SizeOf(Pattern) + 1);

  if Addr = nil then
  begin
    Result := False;
    Exit;
  end;

  if not CheckByte(Addr, $68, 0) then
  begin
    Result := False;
    Exit;
  end
  else
  begin
    Addr := Transpose(Addr, 1);
    pBUFFERSIZE := Addr;
    BUFFERSIZE_DEF := PCardinal(Addr)^;
  end;

  Result := True;
end;

function Find_MAXFLEXCONTROLLER: Boolean;
const
  Pattern: array[0..7] of Byte = ($81, $FA, $80, $00, $00, $00, $0F, $8D);
var
  Addr: Pointer;
begin
  Addr := FindPattern(Pointer(Base), Size, @Pattern[0], SizeOf(Pattern), 2);

  if Addr = nil then
  begin
    Result := False;
    Exit;
  end;

  pMAXFLEXCONTROLLER := Addr;
  MAXFLEXCONTROLLER_DEF := PCardinal(Addr)^;

  Result := True;
end;

function Find_MAXSTUDIOVERTS: Boolean;
var
  Addr: Pointer;
begin
  Addr := FindPushString(Pointer(Base), Size, 'Too many unified vertices');
  if Addr = nil then
  begin
    Result := False;
    Exit;
  end;

  if not CheckByte(Addr, $81, -12) then
  begin
    Result := False;
    Exit;
  end;

  Addr := Pointer(Integer(Addr) - 6);
  pMAXSTUDIOVERTS := Addr;
  MAXSTUDIOVERTS_DEF := PCardinal(Addr)^;

  Result := True;
end;

function Find_VList: Boolean;
var
  Addr: Pointer;
begin
  Addr := @pfnAddToVlist;

  if Addr = nil then
  begin
    Result := False;
    Exit;
  end;

  Addr := FindWordPtr(Addr, 64, $048B, 3);

  if Addr = nil then
  begin
    Result := False;
    Exit;
  end;

  pVList := PPointer(Addr)^;

  Result := True;
end;

function Find_AddrToVlist: Boolean;
var
  Addr: Pointer;
begin
  Addr := FindPushString(Pointer(Base), Size, 'Too many unified vertices'#10);

  if Addr = nil then
  begin
    Result := False;
    Exit;
  end;

  Addr := FindWordPtr(Addr, 256, $EC8B, -1, True);
  if Addr <> nil then
    pfnAddToVlist := Addr
  else
  begin
    Result := False;
    Exit;
  end;

  Result := True;
end;

function Hook_VerticesPtrs: Boolean;
begin
  SetLength(VerticesPtrsNew, MAXSTUDIOVERTS_NEW);

  if HookRefAddr(Pointer(Base), Size, pVList, @VerticesPtrsNew[0], True) = 0 then
  begin
    SetLength(VerticesPtrsNew, 0);
    Result := False;
    Exit;
  end;

  Result := True;
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
    Result := True;
  end
  else
    Result := False;
end;

function Patch_MAXFLEXCONTROLLER: Boolean;
begin
  if pMAXFLEXCONTROLLER = nil then
    Result := False
  else
  begin
    WriteLong(pMAXFLEXCONTROLLER, MAXFLEXCONTROLLER_NEW);
    Result := True;
  end;
end;

function Hook_FlexController: Boolean;
const
  Pattern: array[0..10] of Byte = ($69, $C0, $08, $01, $00, $00, $68, $80, $00, $00, $00);
var
  Addr: Pointer;
begin
  Addr := FindPattern(Pointer(Base), Size, @Pattern[0], SizeOf(Pattern));
  Addr := FindBytePtr(Addr, 128, $05, 1);

  if Addr <> nil then
  begin
    SetLength(FlexControllerNew, MAXFLEXCONTROLLER_NEW);

    Addr := PPointer(Addr)^;

    if HookRefAddr(Pointer(Base), Size, Addr, @FlexControllerNew[0], True) = 0 then
    begin
      SetLength(FlexControllerNew, 0);
      Result := False;
    end
    else
      Result := True;

    Exit;
  end;

  Result := False;
end;

function Hook_IsInt24: Boolean;
begin
  Result := HookRefCall(Pointer(Base), Size, @pIsInt24, @IsInt24) <> 0;
end;

end.

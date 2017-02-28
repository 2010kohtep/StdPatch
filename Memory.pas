unit Memory;

{$DEFINE SAFECODE}
{$DEFINE FATAL}

{$IF COMPILERVERSION >= 17}
  {$DEFINE INLINE}
{$IFEND}

interface

uses
  SysUtils, Windows, Types;

function Absolute(Addr: Pointer): Pointer; {$IFDEF INLINE} inline; {$ENDIF}
function Relative(NewFunc, Address: Pointer): Pointer; {$IFDEF INLINE} inline; {$ENDIF}
function Transpose(Addr: Pointer; Offset: Integer): Pointer; {$IFDEF INLINE} inline; {$ENDIF}

function AllocExecMem(Size: Cardinal): Pointer;
procedure SetNopes(Addr: Pointer; Count: Cardinal);
function SetProtect(Addr: Pointer; Protect: Cardinal; Size: Integer): Cardinal;

function GetModuleSize(Module: HMODULE): Integer;
function GetSectionPtr(Module: Pointer; Name: PAnsiChar): PImageSectionHeader;
function GetSectionSize(Module: Pointer; Name: PAnsiChar): Integer;

function WriteLStr(Addr: Pointer; const Value: AnsiString): Pointer; {$IFDEF INLINE} inline; {$ENDIF}
function WriteWStr(Addr: Pointer; Value: PWideChar): Pointer;
function WriteCStr(Addr: Pointer; Value: PAnsiChar): Pointer;
function WriteBuffer(Addr: Pointer; Buffer: Pointer; Size: Integer): Pointer;
function WriteInt64(Addr: Pointer; Value: Int64): Pointer;
function WritePointer(Addr, Value: Pointer): Pointer; {$IFDEF INLINE} inline; {$ENDIF}
function WriteDouble(Addr: Pointer; Value: Double): Pointer;
function WriteFloat(Addr: Pointer; Value: Single): Pointer;
function WriteLong(Addr: Pointer; Value: Cardinal): Pointer;
function WriteWord(Addr: Pointer; Value: Word): Pointer;
function WriteByte(Addr: Pointer; Value: Byte): Pointer;

function FindBytePtr(Start: Pointer; Size: Cardinal; Value: Byte; Offset: Integer = 0; Back: Boolean = False): Pointer;
function FindWordPtr(Start: Pointer; Size: Cardinal; Value: Word; Offset: Integer = 0; Back: Boolean = False): Pointer;
function FindLongPtr(Start: Pointer; Size: Cardinal; Value: Cardinal; Offset: Integer = 0; Back: Boolean = False): Pointer;
function FindPattern(Addr: Pointer; Len: Cardinal; Pattern: Pointer; PatternSize: Cardinal; Offset: Integer = 0): Pointer;
function FindPushString(Addr: Pointer; Len: Cardinal; Str: PAnsiChar): Pointer;

procedure InsertFunc(From, Dest: Pointer; IsCall: Boolean);
procedure InsertCall(From, Dest: Pointer); {$IFDEF INLINE} inline; {$ENDIF}
procedure InsertJump(From, Dest: Pointer); {$IFDEF INLINE} inline; {$ENDIF}

function CheckByte(Addr: Pointer; Value: Byte; Offset: Integer = 0): Boolean; {$IFDEF INLINE} inline; {$ENDIF}
function CheckWord(Addr: Pointer; Value: Word; Offset: Integer = 0): Boolean; {$IFDEF INLINE} inline; {$ENDIF}
function CheckLong(Addr: Pointer; Value: Cardinal; Offset: Integer = 0): Boolean; {$IFDEF INLINE} inline; {$ENDIF}

function Bounds(AddrStart, AddrEnd: Pointer; Addr: Pointer): Boolean; {$IFDEF INLINE} inline; {$ENDIF}

function FindRefAddr(Addr: Pointer; Len: Cardinal; Ref: Pointer; IgnoreHdr: Boolean; Hdr: Byte): Pointer;
function FindRefCall(Addr: Pointer; Len: Cardinal; Ref: Pointer): Pointer; {$IFDEF INLINE} inline; {$ENDIF}
function FindRefJump(Addr: Pointer; Len: Cardinal; Ref: Pointer): Pointer; {$IFDEF INLINE} inline; {$ENDIF}
function HookRefAddr(Addr: Pointer; Len: Cardinal; Ref, NewRef: Pointer; IgnoreHdr: Boolean; Hdr: Byte = $00): Cardinal;
function HookRefCall(Addr: Pointer; Len: Cardinal; Ref, NewRef: Pointer): Cardinal; {$IFDEF INLINE} inline; {$ENDIF}
function HookRefJump(Addr: Pointer; Len: Cardinal; Ref, NewRef: Pointer): Cardinal; {$IFDEF INLINE} inline; {$ENDIF}

function FindAddr(Addr: Pointer; Offset: Integer; Back: Boolean; Hdr: Byte): Pointer;
function FindAddrEx(Addr: Pointer; Back: Boolean; Hdr: Byte): Pointer; {$IFDEF INLINE} inline; {$ENDIF}
function FindNextAddr(Addr: Pointer; Number: Cardinal; Offset: Integer = 0; Back: Boolean = False; Hdr: Byte = $E8): Pointer;
function FindNextCall(Addr: Pointer; Number: Cardinal; Offset: Integer = 0; Back: Boolean = False): Pointer;
function FindNextCallEx(Addr: Pointer; Back: Boolean = False): Pointer; {$IFDEF INLINE} inline; {$ENDIF}
function FindNextJump(Addr: Pointer; Number: Cardinal; Offset: Integer = 0; Back: Boolean = False): Pointer;
function FindNextJumpEx(Addr: Pointer; Back: Boolean = False): Pointer; {$IFDEF INLINE} inline; {$ENDIF}

function HookWinAPI(OldFunc, NewFunc: Pointer): Pointer;

procedure Crash(const Func: string);

implementation

function StrLen(S: Pointer; Wide: Boolean): Integer;
var
  P: Pointer;
begin
  if (S = nil) or (PByte(S)^ = 0) then
  begin
    Result := 0;
    Exit;
  end;

  P := S;

  if Wide then while PWord(P)^ <> $0000 do Inc(Integer(P), SizeOf(WideChar))
  else while PByte(P)^ <> $00 do Inc(Integer(P), SizeOf(AnsiChar));

  Result := Integer(P) - Integer(S);
end;

function StrCopy(Dest, Source: Pointer; Wide: Boolean): Pointer;
var
  I: Integer;
begin
  if Wide then
  begin
    I := StrLen(Source, True);
    Move(Source^, Dest^, I + SizeOf(WideChar));
    Result := @PWideChar(Dest)[I];
  end
  else
  begin
    I := StrLen(Source, False);
    Move(Source^, Dest^, I + SizeOf(AnsiChar));
    Result := @PAnsiChar(Dest)[I];
  end;
end;

function StrIdent(S1, S2: PAnsiChar): Boolean;
begin
  Result := CompareMem(S1, S2, StrLen(S1, False));
end;

procedure Crash(const Func: string);
var
  Buf: array[0..511] of Char;
begin
  StrFmt(Buf, '%s: Fatal error.', [Func]);
  MessageBox(HWND_DESKTOP, Buf, 'Fatal Error', MB_ICONERROR or MB_SYSTEMMODAL);
  ExitProcess(0);
end;

function Absolute(Addr: Pointer): Pointer;
begin
  Result := Pointer(Integer(Addr) + PInteger(Addr)^ + SizeOf(Pointer));
end;

function Relative(NewFunc, Address: Pointer): Pointer;
begin
  Result := Pointer(Integer(NewFunc) - Integer(Address) - SizeOf(Pointer));
end;

function Transpose(Addr: Pointer; Offset: Integer): Pointer;
begin
  Result := Pointer(Integer(Addr) + Offset);
end;

function AllocExecMem(Size: Cardinal): Pointer;
var
  OldProtect: Cardinal;
begin
  GetMem(Result, Size);
  VirtualProtect(Result, Size, PAGE_EXECUTE_READWRITE, OldProtect);
end;

procedure SetNopes(Addr: Pointer; Count: Cardinal);
var
  Protect: LongWord;
begin
  {$IFDEF SAFECODE}
  if (Addr = nil) or (Count = 0) then
    {$IFDEF FATAL} Crash('SetNopes'); {$ELSE} Exit; {$ENDIF}
  {$ENDIF}

  Protect := SetProtect(Addr, PAGE_EXECUTE_READWRITE, Count);
  FillChar(Addr^, Count, $90);
  SetProtect(Addr, Protect, Count);
end;

function GetModuleSize(Module: HMODULE): Integer;
var
  DOS: PImageDosHeader;
  NT: PImageNtHeaders;
begin
  DOS := Pointer(Module);
  NT := PImageNtHeaders(Integer(DOS) + DOS._lfanew);

  Result := NT^.OptionalHeader.SizeOfImage;
end;

function GetSectionPtr(Module: Pointer; Name: PAnsiChar): PImageSectionHeader;
type
  PImageSectionHeaderArray = ^TImageSectionHeaderArray;
  TImageSectionHeaderArray = array of TImageSectionHeader;
var
  DOS: PImageDosHeader;
  NT: PImageNtHeaders;
  I: Integer;
begin
  DOS := Module;
  NT := PImageNtHeaders(Integer(DOS) + DOS._lfanew);

  Result := PImageSectionHeader(Integer(@NT.OptionalHeader) + NT^.FileHeader.SizeOfOptionalHeader);

  for I := 0 to NT^.FileHeader.NumberOfSections - 1 do
  begin
    if not StrIdent(@Result.Name, Name) then
      Exit;

    Inc(Result, SizeOf(Result^));
  end;

  Result := nil;
end;

function GetSectionSize(Module: Pointer; Name: PAnsiChar): Integer;
var
  Section: PImageSectionHeader;
begin
  Section := GetSectionPtr(Module, Name);

  if Section = nil then
    Result := 0
  else
    Result := Section^.SizeOfRawData;
end;

function SetProtect(Addr: Pointer; Protect: Cardinal; Size: Integer): Cardinal;
begin
  VirtualProtect(Addr, Size, Protect, Result);
end;

function WriteUStr(Addr: Pointer; const Value: WideString): Pointer;
begin
  Result := WriteWStr(Addr, PWideChar(Value));
end;

function WriteLStr(Addr: Pointer; const Value: AnsiString): Pointer;
begin
  Result := WriteCStr(Addr, PAnsiChar(Value));
end;

function WriteWStr(Addr: Pointer; Value: PWideChar): Pointer;
var
  Len: Integer;
  Old: Cardinal;
begin
  {$IFDEF SAFECODE}
  if (Addr = nil) or (Value = nil) or (Value^ = #0) then
    {$IFDEF FATAL} Crash('WriteWStr'); {$ELSE} begin Result := nil; Exit; end; {$ENDIF}
  {$ENDIF}

  Len := (StrLen(Value, True) * SizeOf(WideChar(#0))) + SizeOf(WideChar(#0));

  Old := SetProtect(Addr, PAGE_EXECUTE_READWRITE, Len);
  StrCopy(Addr, Value, True);
  SetProtect(Addr, Old, Len);

  Result := @TByteDynArray(Addr)[Len];
end;

function WriteCStr(Addr: Pointer; Value: PAnsiChar): Pointer;
var
  Len: Integer;
  Old: Cardinal;
begin
  {$IFDEF SAFECODE}
  if (Addr = nil) or (Value = nil) or (Value^ = #0) then
    {$IFDEF FATAL} Crash('WriteCStr'); {$ELSE} begin Result := nil; Exit; end; {$ENDIF}
  {$ENDIF}

  Len := StrLen(Value, False) + SizeOf(AnsiChar(#0));

  Old := SetProtect(Addr, PAGE_EXECUTE_READWRITE, Len);
  StrCopy(Addr, Value, False);
  SetProtect(Addr, Old, Len);

  Result := @TByteDynArray(Addr)[Len];
end;

function WriteBuffer(Addr: Pointer; Buffer: Pointer; Size: Integer): Pointer;
var
  Old: Cardinal;
begin
  {$IFDEF SAFECODE}
  if (Addr = nil) or (Buffer = nil) or (Size <= 0) then
    {$IFDEF FATAL} Crash('WriteBuffer'); {$ELSE} begin Result := nil; Exit; end; {$ENDIF}
  {$ENDIF}

  Old := SetProtect(Addr, PAGE_EXECUTE_READWRITE, Size);
  Move(Addr^, Buffer^, Size);
  SetProtect(Addr, Old, Size);

  Result := @TByteDynArray(Addr)[Size];
end;

function WriteInt64(Addr: Pointer; Value: Int64): Pointer;
var
  Protect: Cardinal;
begin
  {$IFDEF SAFECODE}
  if Addr = nil then
    {$IFDEF FATAL} Crash('WriteInt64'); {$ELSE} begin Result := nil; Exit; end; {$ENDIF}
  {$ENDIF}

  Protect := SetProtect(Addr, PAGE_EXECUTE_READWRITE, SizeOf(Value));
  PInt64(Addr)^ := Value;
  SetProtect(Addr, Protect, SizeOf(Value));
  Result := Pointer(LongWord(Addr) + SizeOf(Value));
end;

function WritePointer(Addr, Value: Pointer): Pointer;
begin
  Result := WriteLong(Addr, Cardinal(Value));
end;

function WriteDouble(Addr: Pointer; Value: Double): Pointer;
var
  Old: Cardinal;
begin
  {$IFDEF SAFECODE}
  if Addr = nil then {$IFDEF FATAL} Crash('WriteDouble'); {$ELSE} begin Result := nil; Exit; end; {$ENDIF}
  {$ENDIF}

  Old := SetProtect(Addr, PAGE_EXECUTE_READWRITE, SizeOf(Value));
  PDouble(Addr)^ := Value;
  SetProtect(Addr, Old, SizeOf(Value));

  Result := @TByteDynArray(Addr)[SizeOf(Value)];
end;

function WriteFloat(Addr: Pointer; Value: Single): Pointer;
var
  Old: Cardinal;
begin
  {$IFDEF SAFECODE}
  if Addr = nil then {$IFDEF FATAL} Crash('WriteFloat'); {$ELSE} begin Result := nil; Exit; end; {$ENDIF}
  {$ENDIF}

  Old := SetProtect(Addr, PAGE_EXECUTE_READWRITE, SizeOf(Value));
  PSingle(Addr)^ := Value;
  SetProtect(Addr, Old, SizeOf(Value));

  Result := @TByteDynArray(Addr)[SizeOf(Value)];
end;

function WriteLong(Addr: Pointer; Value: Cardinal): Pointer;
var
  Old: Cardinal;
begin
  {$IFDEF SAFECODE}
  if Addr = nil then {$IFDEF FATAL} Crash('WriteLong'); {$ELSE} begin Result := nil; Exit; end; {$ENDIF}
  {$ENDIF}

  Old := SetProtect(Addr, PAGE_EXECUTE_READWRITE, SizeOf(Value));
  PCardinal(Addr)^ := Value;
  SetProtect(Addr, Old, SizeOf(Value));

  Result := @TByteDynArray(Addr)[SizeOf(Value)];
end;

function WriteWord(Addr: Pointer; Value: Word): Pointer;
var
  Old: Cardinal;
begin
  {$IFDEF SAFECODE}
  if Addr = nil then {$IFDEF FATAL} Crash('WriteWord'); {$ELSE} begin Result := nil; Exit; end; {$ENDIF}
  {$ENDIF}

  Old := SetProtect(Addr, PAGE_EXECUTE_READWRITE, SizeOf(Value));
  PWord(Addr)^ := Value;
  SetProtect(Addr, Old, SizeOf(Value));

  Result := @TByteDynArray(Addr)[SizeOf(Value)];
end;

function WriteByte(Addr: Pointer; Value: Byte): Pointer;
var
  Old: Cardinal;
begin
  {$IFDEF SAFECODE}
  if Addr = nil then
    {$IFDEF FATAL}
    Crash('WriteByte');
    {$ELSE}
    begin Result := nil; Exit; end;
    {$ENDIF}
  {$ENDIF}

  Old := SetProtect(Addr, PAGE_EXECUTE_READWRITE, SizeOf(Value));
  PByte(Addr)^ := Value;
  SetProtect(Addr, Old, SizeOf(Value));

  Result := @TByteDynArray(Addr)[SizeOf(Value)];
end;

function MemInc(Addr: Pointer): Pointer; begin Result := Pointer(Integer(Addr) + 1) end;
function MemDec(Addr: Pointer): Pointer; begin Result := Pointer(Integer(Addr) - 1) end;

function FindBytePtr(Start: Pointer; Size: Cardinal; Value: Byte; Offset: Integer; Back: Boolean): Pointer;
var
  F: function(Addr: Pointer): Pointer;
begin
  {$IFDEF SAFECODE}
  if (Start = nil) or (Size = 0) then
    {$IFDEF FATAL} Crash('FindBytePtr'); {$ELSE} begin Result := nil; Exit; end; {$ENDIF}
  {$ENDIF}

  if Back then F := @MemDec else F := @MemInc;

  while Size > 0 do
  begin
    if PByte(Start)^ = Value then
    begin
      Result := Pointer(Integer(Start) + Offset);
      Exit;
    end;

    Start := F(Start);
    Dec(Size);
  end;

  Result := nil;
  Exit;
end;

function FindWordPtr(Start: Pointer; Size: Cardinal; Value: Word; Offset: Integer; Back: Boolean): Pointer;
var
  F: function(Addr: Pointer): Pointer;
begin
  {$IFDEF SAFECODE}
  if (Start = nil) or (Size = 0) then
    {$IFDEF FATAL} Crash('FindWordPtr'); {$ELSE} begin Result := nil; Exit; end; {$ENDIF}
  {$ENDIF}

  if Back then F := @MemDec else F := @MemInc;

  while Size > 0 do
  begin
    if PWord(Start)^ = Value then
    begin
      Result := Pointer(Integer(Start) + Offset);
      Exit;
    end;

    Start := F(Start);
    Dec(Size);
  end;

  Result := nil;
  Exit;
end;

function FindLongPtr(Start: Pointer; Size: Cardinal; Value: Cardinal; Offset: Integer; Back: Boolean): Pointer;
var
  F: function(Addr: Pointer): Pointer;
begin
  {$IFDEF SAFECODE}
  if (Start = nil) or (Size = 0) then
    {$IFDEF FATAL} Crash('FindLongPtr'); {$ELSE} begin Result := nil; Exit; end; {$ENDIF}
  {$ENDIF}

  if Back then F := @MemDec else F := @MemInc;

  while Size > 0 do
  begin
    if PCardinal(Start)^ = Value then
    begin
      Result := Pointer(Integer(Start) + Offset);
      Exit;
    end;

    Start := F(Start);
    Dec(Size);
  end;

  Result := nil;
  Exit;
end;

function CompareMemory(Dest, Source: PByte; Len: Integer): Boolean;
var
  I: Integer;
begin
  for I := 0 to Len - 1 do
    if (TByteDynArray(Source)[I] <> $FF) and
      (TByteDynArray(Dest)[I] <> TByteDynArray(Source)[I]) then
    begin
      Result := False;
      Exit;
    end;

  Result := True;
end;

function FindPattern(Addr: Pointer; Len: Cardinal; Pattern: Pointer; PatternSize: Cardinal; Offset: Integer): Pointer;
var
  AddrEnd: Pointer;
begin
  {$IFDEF SAFECODE}
  if (Addr = nil) or (Len = 0) or (Pattern = nil) or (PatternSize = 0) then
    {$IFDEF FATAL} Crash('FindPattern'); {$ELSE} begin Result := nil; Exit; end; {$ENDIF}
  {$ENDIF}

  AddrEnd := Pointer(Cardinal(Addr) + Len);

  while Addr <> AddrEnd do
  begin
    if (PByte(Addr)^ = PByte(Pattern)^) and CompareMemory(Addr, Pattern, PatternSize) then
    begin
      Result := Pointer(Integer(Addr) + Offset);
      Exit;
    end;

    Inc(Cardinal(Addr), 1);
  end;

  Result := nil;
end;

function FindPushString(Addr: Pointer; Len: Cardinal; Str: PAnsiChar): Pointer;
var
  P: Pointer;
  A: array[0..4] of Byte;
begin
  {$IFDEF SAFECODE}
  if (Addr = nil) or (Len = 0) or (Str = nil) or (Str^ = #0) then
    {$IFDEF FATAL} Crash('FindPushString'); {$ELSE} begin Result := nil; Exit; end; {$ENDIF}
  {$ENDIF}

  P := FindPattern(Addr, Len, Str, StrLen(Str, False), 0);

  if P = nil then
  begin
    Result := nil;
    Exit;
  end;

  A[0] := $68;
  PPointer(@A[1])^ := P;

  Result := FindPattern(Addr, Len, @A[0], SizeOf(A), 0);
end;

procedure InsertFunc(From, Dest: Pointer; IsCall: Boolean);
var
  B: Byte;
begin
  if IsCall then B := $E8 else B := $E9;

  WriteByte(From, B);
  WriteLong(Transpose(From, 1), Cardinal(Relative(Transpose(Dest, -1), From)));
end;

procedure InsertCall(From, Dest: Pointer);
begin
  InsertFunc(From, Dest, True);
end;

procedure InsertJump(From, Dest: Pointer);
begin
  InsertFunc(From, Dest, False);
end;

function CheckByte(Addr: Pointer; Value: Byte; Offset: Integer): Boolean;
begin
  {$IFDEF SAFECODE}
  if Addr = nil then {$IFDEF FATAL} Crash('CheckByte'); {$ELSE} begin Result := False; Exit; end; {$ENDIF}
  {$ENDIF}

  Result := TByteDynArray(Addr)[Offset] = Value;
end;

function CheckWord(Addr: Pointer; Value: Word; Offset: Integer): Boolean;
begin
  {$IFDEF SAFECODE}
  if Addr = nil then {$IFDEF FATAL} Crash('CheckWord'); {$ELSE} begin Result := False; Exit; end; {$ENDIF}
  {$ENDIF}

  Result := PWord(@TByteDynArray(Addr)[Offset])^ = Value;
end;

function CheckLong(Addr: Pointer; Value: Cardinal; Offset: Integer): Boolean;
begin
  {$IFDEF SAFECODE}
  if Addr = nil then {$IFDEF FATAL} Crash('CheckLong'); {$ELSE} begin Result := False; Exit; end; {$ENDIF}
  {$ENDIF}

  Result := PCardinal(@TByteDynArray(Addr)[Offset])^ = Value;
end;

function Bounds(AddrStart, AddrEnd: Pointer; Addr: Pointer): Boolean;
begin
  Result := (Integer(Addr) < Integer(AddrStart)) or (Integer(Addr) > Integer(AddrEnd));
end;

function FindRefAddr(Addr: Pointer; Len: Cardinal; Ref: Pointer; IgnoreHdr: Boolean; Hdr: Byte): Pointer;
var
  AddrEnd: Pointer;
begin
  {$IFDEF SAFECODE}
  if (Addr = nil) or (Len = 0) or (Ref = nil) then
    {$IFDEF FATAL} Crash('FindRefAddr'); {$ELSE} begin Result := nil; Exit; end; {$ENDIF}
  {$ENDIF}

  AddrEnd := Pointer(Cardinal(Addr) + Len - 3);

  while Addr <> AddrEnd do
  begin
    if IgnoreHdr then
    begin
      if PPointer(Addr)^ = Ref then
      begin
        Result := Addr;
        Exit;
      end
    end
    else
    if (PByte(Addr)^ = Hdr) and (Absolute(Pointer(Cardinal(Addr) + 1)) = Ref) then
    begin
      Result := Addr;
      Exit;
    end;

    Inc(Cardinal(Addr), 1);
  end;

  Result := nil;
end;

function FindRefCall(Addr: Pointer; Len: Cardinal; Ref: Pointer): Pointer;
begin
  Result := FindRefAddr(Addr, Len, Ref, False, $E8);
end;

function FindRefJump(Addr: Pointer; Len: Cardinal; Ref: Pointer): Pointer;
begin
  Result := FindRefAddr(Addr, Len, Ref, False, $E9);
end;

function HookRefAddr(Addr: Pointer; Len: Cardinal; Ref, NewRef: Pointer; IgnoreHdr: Boolean; Hdr: Byte): Cardinal;
var
  P: Pointer;
begin
  {$IFDEF SAFECODE}
  if (Addr = nil) or (Len = 0) or (Ref = nil) or (NewRef = nil) then
    {$IFDEF FATAL} Crash('HookRefAddr'); {$ELSE} begin Result := 0; Exit; end; {$ENDIF}
  {$ENDIF}

  Result := 0;
  repeat
    P := FindRefAddr(Addr, Len, Ref, IgnoreHdr, Hdr);

    if P = nil then
      Exit;

    if IgnoreHdr then
      WritePointer(P, NewRef)
    else
      InsertFunc(P, NewRef, Hdr = $E8);

    Inc(Result);
  until False;
end;

function HookRefCall(Addr: Pointer; Len: Cardinal; Ref, NewRef: Pointer): Cardinal;
begin
  Result := HookRefAddr(Addr, Len, Ref, NewRef, False, $E8);
end;

function HookRefJump(Addr: Pointer; Len: Cardinal; Ref, NewRef: Pointer): Cardinal;
begin
  Result := HookRefAddr(Addr, Len, Ref, NewRef, False, $E9);
end;

function FindAddr(Addr: Pointer; Offset: Integer; Back: Boolean; Hdr: Byte): Pointer;
var
  F: function(Addr: Pointer): Pointer;
  BaseAddr: Pointer;
  MemInfo: TMemoryBasicInformation;
  P, P2: Pointer;
begin
  {$IFDEF SAFECODE}
  if Addr = nil then {$IFDEF FATAL} Crash('FindAddr'); {$ELSE} begin Result := nil; Exit; end; {$ENDIF}
  {$ENDIF}

  if Back then F := @MemDec else F := @MemInc;

  if VirtualQuery(Addr, MemInfo, SizeOf(MemInfo)) = 0 then
  begin
    Result := nil;
    Exit;
  end;

  BaseAddr := MemInfo.AllocationBase;
  P := Addr;

  repeat
    P := FindBytePtr(P, Cardinal(-1), Hdr, 0, Back);

    P2 := Absolute(Transpose(P, 1));
    if VirtualQuery(P2, MemInfo, SizeOf(MemInfo)) <> 0 then
    begin
      if MemInfo.AllocationBase = BaseAddr then
      begin
        Result := Pointer(Integer(P) + Offset);
        Exit;
      end
      else
        P := F(P);
    end;
  until False;

  Result := nil;
end;

function FindAddrEx(Addr: Pointer; Back: Boolean; Hdr: Byte): Pointer;
begin
  Result := FindAddr(Addr, 1, Back, Hdr);

  if Result <> nil then
    Result := Absolute(Result);
end;

function FindNextAddr(Addr: Pointer; Number: Cardinal; Offset: Integer; Back: Boolean; Hdr: Byte): Pointer;
var
  F: function(Addr: Pointer): Pointer;
  OffsetTo: Integer;
begin
  {$IFDEF SAFECODE}
  if (Addr = nil) or (Number = 0) then {$IFDEF FATAL} Crash('FindNextAddr'); {$ELSE} begin Result := nil; Exit; end; {$ENDIF}
  {$ENDIF}

  if Back then OffsetTo := -1 else OffsetTo := 1;
  if not Back then F := @MemDec else F := @MemInc;

  Result := Addr;
  repeat
    Result := FindAddr(Result, OffsetTo, Back, Hdr);

    if Result = nil then
      Exit;

    Dec(Number);
    if Number = 0 then
    begin
      Transpose(F(Result), Offset);
      Exit;
    end;
  until False;

  Result := nil;
end;

function FindNextCall(Addr: Pointer; Number: Cardinal; Offset: Integer; Back: Boolean): Pointer;
begin
  Result := FindNextAddr(Addr, Number, Offset, Back, $E8);
end;

function FindNextCallEx(Addr: Pointer; Back: Boolean): Pointer;
begin
  Result := FindAddrEx(Addr, Back, $E8);
end;

function FindNextJump(Addr: Pointer; Number: Cardinal; Offset: Integer; Back: Boolean): Pointer;
begin
  Result := FindNextAddr(Addr, Number, Offset, Back, $E9);
end;

function FindNextJumpEx(Addr: Pointer; Back: Boolean): Pointer;
begin
  Result := FindAddrEx(Addr, Back, $E9);
end;

function HookWinAPI(OldFunc, NewFunc: Pointer): Pointer;
const
  WINAPI_HEADER: array[0..4] of Byte = ($8B, $FF,  // mov edi, edi
                                        $55,       // push ebp
                                        $8B, $EC); // mov ebp, esp
var
  Addr: Pointer;
begin
  if (OldFunc = nil) or (NewFunc = nil) then
    Result := nil
  else
  begin
    if PWord(OldFunc)^ = $25FF then
    begin
      Addr := PPointer(LongWord(OldFunc) + 2)^;
      Addr := PPointer(Addr)^;
    end
    else
      Addr := OldFunc;

   if (PLongWord(Addr)^ <> PLongWord(@WINAPI_HEADER[0])^) then
      Result := nil
    else
    begin
      InsertJump(Addr, NewFunc);
      Result := AllocExecMem(8);
      PLongWord(Result)^ := $90E58955;
      InsertJump(Pointer(LongWord(Result) + 3), Pointer(LongWord(Addr) + 5));
    end;
  end;
end;

end.

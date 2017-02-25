unit Studio.Funcs;

interface

uses
  SysUtils, Windows, Studio.Global;

procedure MdlError(Msg: PAnsiChar); cdecl;
function AddToVlist(v, m, n, t: Integer): Pointer; cdecl;
function IsInt24(Value: Integer): Integer; cdecl;

implementation

procedure MdlError(Msg: PAnsiChar); cdecl;
begin
  WriteLn('Mismarked Bone flag, but screw this :^)');
end;

function AddToVlist(v, m, n, t: Integer): Pointer; cdecl;
begin
  if (v < 0) or (v > MAXSTUDIOVERTS_NEW) then
  begin
    WriteLn('AddToVlist(): Out of range - Cell: ', v, '; Available: ', Low(VerticesPtrsNew), '..', High(VerticesPtrsNew));
  end;

//  if v mod 256 = 0 then
//    WriteLn(GetCurrentThreadId, ' AddToVlist(', v, ', ', m, ', ', n, ', ', t, ')');

  Result := pfnAddToVlist(v, m, n, t);
end;

function IsInt24(Value: Integer): Integer; cdecl;
begin
  if (Value < -$800000) or (Value > $7FFFFF) then
    WriteLn('IsInt24(', Value, '): Bad value.');

  Result := Value;
end;

end.

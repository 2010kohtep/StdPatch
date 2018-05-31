unit Studio.Funcs;

interface

uses
  SysUtils, Windows, Studio.Global;

procedure MdlError(Msg: PAnsiChar); cdecl;
function IsInt24(Value: Integer): Integer; cdecl;

implementation

procedure MdlError(Msg: PAnsiChar); cdecl;
begin
  WriteLn('Mismarked Bone flag, but screw this :^)');
end;

function IsInt24(Value: Integer): Integer; cdecl;
begin
  if (Value < -$800000) or (Value > $7FFFFF) then
    WriteLn('IsInt24(', Value, '): Bad value.');

  Result := Value;
end;

end.

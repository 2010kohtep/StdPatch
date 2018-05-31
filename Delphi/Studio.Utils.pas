unit Studio.Utils;

interface

uses
  SysUtils;

function BytesToStr(Value: LongWord): string;
function IfThen(AValue: Boolean; ATrue: PAnsiChar; AFalse: PAnsiChar): PAnsiChar;

implementation

function BytesToStr(Value: LongWord): string;
var
  Prev: LongWord;
begin
  Prev := Value;
  Value := Value div 1024;

  if Value = 0 then
  begin
    Result := IntToStr(Prev) + ' bytes';
    Exit;
  end;

  Prev := Value;
  Value := Value div 1024;
  if Value = 0 then
  begin
    Result := IntToStr(Prev) + ' kbytes';
    Exit;
  end;

  Prev := Value;
  Value := Value div 1024;
  if Value = 0 then
  begin
    Result := IntToStr(Prev) + ' mbytes';
    Exit;
  end;

  Result := IntToStr(Value) + ' gbytes';
end;

function IfThen(AValue: Boolean; ATrue: PAnsiChar; AFalse: PAnsiChar): PAnsiChar;
begin
  if AValue then
    Result := ATrue
  else
    Result := AFalse;
end;

end.

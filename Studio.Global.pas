unit Studio.Global;

interface

uses
  Studio.SDK;

var
  MAXSTUDIOVERTS_NEW: LongInt = $00F00000; // def - $80000
  BUFFERSIZE_NEW: LongInt = $2000000;

var
  MAXSTUDIOVERTS_DEF: LongWord;
  BUFFERSIZE_DEF: LongWord;

var
  Base: THandle;
  Size: LongWord;

  VerticesPtrsNew: TPVUnifyArray;
  VerticesDataNew: TVUnifyArray; // PVUnifyArray;
  WeightList: array of TWeightList;

  pBUFFERSIZE, pMAXSTUDIOVERTS: Pointer;
  pfnAddToVlist: function(a1, a2, a3, a4: LongWord): Pointer; cdecl;
  pIsInt24: function(Value: Integer): Boolean; cdecl;
  pVList: Pointer;

implementation

begin

end.

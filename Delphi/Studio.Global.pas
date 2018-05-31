unit Studio.Global;

interface

uses
  Studio.SDK;

var
  MAXSTUDIOVERTS_NEW: LongInt = $00F00000; // def - $80000
  BUFFERSIZE_NEW: LongInt = $2000000; // def - $2000000
  MAXFLEXCONTROLLER_NEW: LongInt = $00000400; // def - $80

var
  MAXSTUDIOVERTS_DEF: Cardinal;
  BUFFERSIZE_DEF: Cardinal;
  MAXFLEXCONTROLLER_DEF: Cardinal;

var
  Base: HMODULE;
  Size: Cardinal;

  FlexControllerNew: TFlexControllerArray;
  VerticesPtrsNew: TPVUnifyArray;
  VerticesDataNew: TVUnifyArray;
  WeightList: array of TWeightList;

  pBUFFERSIZE, pMAXSTUDIOVERTS, pMAXFLEXCONTROLLER: Pointer;

  pfnAddToVlist: function(a1, a2, a3, a4: Cardinal): Pointer; cdecl;
  pIsInt24: function(Value: Integer): Boolean; cdecl;
  pVList: Pointer;

  pFlexController: Pointer;

implementation

begin

end.

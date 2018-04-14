unit Studio.SDK;

interface

const
  MAXWEIGHTLISTS = 128;
  MAXSTUDIONAME = 128;
  MAXSTUDIOBONES = 128;

  MAXWEIGHTSPERLIST = MAXSTUDIOBONES;

type
  TWeightList = record
    Dymmy: array[0..5252 - 1] of Byte;
  end;

  TFlexController = record
    Name, FlexType: array[0..127] of AnsiChar;
    Min, Max: Single;
  end;

  PVUnify = ^TVUnify;
  TVUnify = record
    RefCount, LastRef, FirstRef: Integer;
    V, M, N, T: Integer;
    Next: PVUnify;
  end;

  PPVUnifyArray = ^TPVUnifyArray;
  TPVUnifyArray = array of PVUnify;

  PVUnifyArray = ^TVUnifyArray;
  TVUnifyArray = array of TVUnify;

  PFlexControllerArray = ^TFlexControllerArray;
  TFlexControllerArray = array of TFlexController;

implementation

end.

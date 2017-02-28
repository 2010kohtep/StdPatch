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

implementation

end.

unit Studio.SDK;

interface

const
  MAXWEIGHTLISTS = 128;
  MAXSTUDIONAME = 128;
  MAXSTUDIOBONES = 128;

  MAXWEIGHTSPERLIST = MAXSTUDIOBONES;

type
  TWeightList = record
    Dump: array[0..5252 - 1] of Byte;
// Name: array[0..MAXSTUDIONAME - 1] of AnsiChar;
// NumBones: LongInt;
// BoneName: array[0..MAXWEIGHTSPERLIST - 1] of PAnsiChar;
// BoneWeight: array[0..MAXWEIGHTSPERLIST - 1] of Single;
// BonePosWeight: array[0..MAXWEIGHTSPERLIST - 1] of Single;
//
// Weight: array[0..MAXWEIGHTSPERLIST - 1] of Single;
// PosWeight: array[0..MAXWEIGHTSPERLIST - 1] of Single;
  end;

  PVUnify = ^TVUnify;
  TVUnify = record
    RefCount, LastRef, FirstRef: LongInt;
    V, M, N, T: LongInt;
    Next: PVUnify;
  end;

  PPVUnifyArray = ^TPVUnifyArray;
  TPVUnifyArray = array of PVUnify;

  PVUnifyArray = ^TVUnifyArray;
  TVUnifyArray = array of TVUnify;

implementation

end.

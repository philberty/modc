MODULE Program2;

VAR
  I,J,K:INTEGER;
  C1,C2:CHAR;
  B1,B2:BOOLEAN;
  A1:ARRAY[1..7] OF INTEGER;
  A2:ARRAY[0..14] OF CHAR;
  A3:ARRAY[22..34] OF BOOLEAN;

BEGIN
  J:=4;
  K:=50;

  I:=J-K;
  K:=I+K;
  J:=J*J;
  A1[1]:=J DIV K;

  B1:=TRUE;
  B2:=FALSE;

  A3[22]:=B1 AND B2;
  A3[23]:=B1 OR B2;
  A3[24]:=(NOT B1);

  A3[25]:=I=J;
  A3[25]:=I<>J;
  A3[25]:=I<J;
  A3[25]:=I<=J;
  A3[25]:=I>J;
  A3[25]:=I>=J

END Program2.

MODULE Program3;

VAR
  I,J,K:INTEGER;
  C1,C2:CHAR;
  B1,B2:BOOLEAN;
  A1:ARRAY[1..7] OF INTEGER;
  A3:ARRAY[22..34] OF BOOLEAN;

BEGIN
  J:=4;
  K:=50;

  IF J>K THEN
    J:=J-1;
    K:=K+1
  ELSE
    K:=K-1
  END;

  FOR J:=1 TO 7 DO
    A1[J]:=0
  END;

  LOOP
    K:=K-1;
    IF K=J THEN
      EXIT
    END
  END;

  CASE K OF
  | 1 :     K:=2
  | 3 :     K:=4
  | 5,6,7 : K:=8
  | 2,4,8 : K:=15
  END

END Program3.

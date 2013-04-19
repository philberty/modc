MODULE Program4;

VAR
  I:BOOLEAN;

  PROCEDURE A;
  VAR I:INTEGER;

    PROCEDURE B;
    VAR I:INTEGER;

    BEGIN
      I:=4
    END B;
  
  BEGIN
    I:=5
  END A;

BEGIN
  I:=TRUE;
  A
END Program4.

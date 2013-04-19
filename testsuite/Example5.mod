MODULE Program5;

VAR
  A1:ARRAY[0..11] OF CHAR;
  J:INTEGER;
  I:CHAR;

BEGIN
  READ(J);
  READ(I);

  A1[0]:='h';  
  A1[1]:='e';  
  A1[2]:='l';  
  A1[3]:='l';  
  A1[4]:='o';  
  A1[5]:=' ';  
  A1[6]:='w';  
  A1[7]:='o';  
  A1[8]:='r';  
  A1[9]:='l';  
  A1[10]:='d';  

  FOR J:=1 TO 11 DO
    WRITE(A1[J])
  END

END Program5.

DECLARE
    c_num CONSTANT NUMBER := 10;
    c_str CONSTANT VARCHAR2(20) := 'Значение: ';
BEGIN
    DBMS_OUTPUT.PUT_LINE(c_str || (c_num * 5));
END;
/
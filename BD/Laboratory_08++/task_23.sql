DECLARE
    v_val NUMBER := 3;
BEGIN
    CASE v_val
        WHEN 1 THEN DBMS_OUTPUT.PUT_LINE('Один');
        WHEN 2 THEN DBMS_OUTPUT.PUT_LINE('Два');
        ELSE DBMS_OUTPUT.PUT_LINE('Ни один, ни два');
    END CASE;
END;
/
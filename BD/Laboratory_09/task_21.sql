DECLARE
    v_val NUMBER := 2;
BEGIN
    IF v_val = 1 THEN
        DBMS_OUTPUT.PUT_LINE('Один');
    ELSIF v_val = 2 THEN
        DBMS_OUTPUT.PUT_LINE('Два');
    ELSE
        DBMS_OUTPUT.PUT_LINE('Ни один, ни два');
    END IF;
END;
/
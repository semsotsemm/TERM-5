DECLARE
    v_origin NUMBER(8, 2) := 99.99;
    v_copy v_origin%TYPE;
BEGIN
    v_copy := v_origin + 10;
    DBMS_OUTPUT.PUT_LINE('Переменная v_copy: ' || v_copy);
END;
/
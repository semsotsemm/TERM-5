DECLARE
    v_row DUAL%ROWTYPE;
BEGIN
    SELECT * INTO v_row FROM DUAL;
    DBMS_OUTPUT.PUT_LINE('Значение столбца dummy из DUAL: ' || v_row.dummy);
END;
/
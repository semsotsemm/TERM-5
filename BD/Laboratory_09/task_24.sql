DECLARE
    i NUMBER := 1;
BEGIN
    LOOP
        DBMS_OUTPUT.PUT_LINE('Итерация цикла: ' || i);
        i := i + 1;
        EXIT WHEN i > 3;
    END LOOP;
END;
/
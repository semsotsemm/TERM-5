DECLARE
    i NUMBER := 1;
BEGIN
    WHILE i <= 3 LOOP
        DBMS_OUTPUT.PUT_LINE('Итерация: ' || i);
        i := i + 1;
    END LOOP;
END;
/
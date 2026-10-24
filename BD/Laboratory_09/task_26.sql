BEGIN
    FOR i IN 1..3 LOOP
        DBMS_OUTPUT.PUT_LINE('Итерация: ' || i);
    END LOOP;
END;
/
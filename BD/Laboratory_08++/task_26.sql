BEGIN
    FOR i IN 1..3 LOOP
        DBMS_OUTPUT.PUT_LINE('Итерация: ' || i);
    END LOOP;
END;
/

-- как сделать step 2, диапазон пользовательских ошибок от какого до какого числа
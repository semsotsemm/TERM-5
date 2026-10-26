DECLARE
    v_num NUMBER;
BEGIN
    v_num := 1 / 0;
    EXCEPTION
    WHEN OTHERS THEN
        DBMS_OUTPUT.PUT_LINE('Код ошибки: ' || SQLCODE);
        DBMS_OUTPUT.PUT_LINE('Сообщение: ' || SQLERRM);
END;
/
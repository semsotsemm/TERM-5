DECLARE 
    pulpits PULPIT%ROWTYPE;
BEGIN
    SELECT * 
        INTO pulpits
        FROM PULPIT pl;
    
    EXCEPTION
        WHEN TOO_MANY_ROWS THEN
            DBMS_OUTPUT.PUT_LINE('-1422: Ошибка в переменую выбрано слишком много значений.');
        WHEN OTHERS THEN
            DBMS_OUTPUT.PUT_LINE(SQLCODE || ': ' || SQLERRM);
END;
/
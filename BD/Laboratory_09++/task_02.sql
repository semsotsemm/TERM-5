DECLARE 
    pulpits PULPIT%ROWTYPE;
BEGIN
    SELECT * 
        INTO pulpits
        FROM PULPIT pl;
    
    EXCEPTION
        WHEN OTHERS THEN
            DBMS_OUTPUT.PUT_LINE(SQLCODE || ': ' || SQLERRM);
END;
/
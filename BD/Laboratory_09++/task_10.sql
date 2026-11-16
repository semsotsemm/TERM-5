BEGIN
    DELETE 
        FROM PULPIT
        WHERE PULPIT = 'ИСиТ';
    
    DBMS_OUTPUT.PUT_LINE('Строка успешно удалена.');
    
    EXCEPTION 
        WHEN OTHERS THEN
            DBMS_OUTPUT.PUT_LINE(SQLCODE || ': Ошибка удаления, нарушение целостности');
END;
/
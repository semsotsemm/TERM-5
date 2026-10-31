DECLARE 
    v_new_value NUMBER(4) := 100;
BEGIN
    UPDATE TEACHER
        SET GENDER = v_new_value
        WHERE TEACHER = 'ГРН';
    
    
    EXCEPTION 
        WHEN OTHERS THEN
            DBMS_OUTPUT.PUT_LINE('Ошибка вставки значения ' || v_new_value || ': нарушение целостности таблицы.');
END;
/
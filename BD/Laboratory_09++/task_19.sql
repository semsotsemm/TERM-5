DECLARE
    v_row_id ROWID;
    v_typename AUDITORIUM_TYPE.AUDITORIUM_TYPENAME%TYPE;
    v_type AUDITORIUM_TYPE.AUDITORIUM_TYPE%TYPE;
BEGIN
    SELECT ROWID, aud_t.AUDITORIUM_TYPENAME, aud_t.AUDITORIUM_TYPE
        INTO v_row_id, v_typename, v_type
        FROM AUDITORIUM_TYPE aud_t
        WHERE AUDITORIUM_TYPENAME = 'Лекционная';
    DBMS_OUTPUT.PUT_LINE('Полученная информация:');
    
    DBMS_OUTPUT.PUT_LINE('ID строки:' || v_row_id);
    DBMS_OUTPUT.PUT_LINE('Аудитория:' || v_type);
    DBMS_OUTPUT.PUT_LINE('Название аудитории:' || v_typename);
    COMMIT;
    
    
    UPDATE AUDITORIUM_TYPE
        SET AUDITORIUM_TYPENAME = 'Компьютерный класс'
        WHERE AUDITORIUM_TYPE.ROWID = v_row_id;

    SELECT ROWID, aud_t.AUDITORIUM_TYPENAME, aud_t.AUDITORIUM_TYPE
        INTO v_row_id, v_typename, v_type
        FROM AUDITORIUM_TYPE aud_t
        WHERE ROWID = v_row_id;
        
    DBMS_OUTPUT.PUT_LINE('После обновления:'); 
    DBMS_OUTPUT.PUT_LINE('ID строки:' || v_row_id);
    DBMS_OUTPUT.PUT_LINE('Аудитория:' || v_type);
    DBMS_OUTPUT.PUT_LINE('Название аудитории:' || v_typename);

    DELETE AUDITORIUM_TYPE
        WHERE ROWID = v_row_id;
    
    
    ROLLBACK;
    DBMS_OUTPUT.PUT_LINE('Таблица возвращена в исходное состояние.');
END;
/
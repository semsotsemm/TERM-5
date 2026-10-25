DECLARE
    v_new_auditorium_type AUDITORIUM_TYPE.AUDITORIUM_TYPENAME%TYPE := 'Новый компьютерный класс';
    v_old_auditorium_type AUDITORIUM_TYPE.AUDITORIUM_TYPENAME%TYPE := 'Спец. компьютерный класс';
    v_auditorium_type_bad_value VARCHAR(30):= 'Ошибка';
    v_value_from_table AUDITORIUM_TYPE%ROWTYPE;
BEGIN
    UPDATE AUDITORIUM_TYPE 
        SET AUDITORIUM_TYPENAME = v_new_auditorium_type
        WHERE AUDITORIUM_TYPENAME = v_old_auditorium_type;

    
    IF SQL%ROWCOUNT = 0 THEN
        DBMS_OUTPUT.PUT_LINE('Не обновлено ни одной строки.');
    ELSE
        DBMS_OUTPUT.PUT_LINE('Успешно обновленно ' || SQL%ROWCOUNT || ' записей');
    END IF;
    ROLLBACK;
    DBMS_OUTPUT.PUT_LINE('Обновление откачено при помощи rollback.');

    
    SELECT *
        INTO v_value_from_table
        FROM AUDITORIUM_TYPE aud_t
        WHERE aud_t.AUDITORIUM_TYPENAME = v_old_auditorium_type; 
    DBMS_OUTPUT.PUT_LINE('Значение из таблицы: ' || v_value_from_table.AUDITORIUM_TYPENAME);
  
  
    
    UPDATE AUDITORIUM_TYPE 
        SET AUDITORIUM_TYPENAME = v_new_auditorium_type
        WHERE AUDITORIUM_TYPENAME = v_old_auditorium_type;


    IF SQL%ROWCOUNT = 0 THEN
        DBMS_OUTPUT.PUT_LINE('Не обновлено ни одной строки.');
    ELSE
        DBMS_OUTPUT.PUT_LINE('Успешно обновленно ' || SQL%ROWCOUNT || ' записей');
    END IF;
    COMMIT;
    DBMS_OUTPUT.PUT_LINE('Изменение успешно зафиксированно в базе данных.');
    
    
    
    SELECT *
        INTO v_value_from_table
        FROM AUDITORIUM_TYPE aud_t
        WHERE aud_t.AUDITORIUM_TYPENAME = v_new_auditorium_type;
    DBMS_OUTPUT.PUT_LINE('Значение из таблицы: ' || v_value_from_table.AUDITORIUM_TYPENAME);
    
    
    
    UPDATE AUDITORIUM_TYPE 
        SET AUDITORIUM_TYPENAME = v_old_auditorium_type
        WHERE AUDITORIUM_TYPENAME = v_new_auditorium_type;
    DBMS_OUTPUT.PUT_LINE('Данные возвращены в исходный вид.');
END;
/
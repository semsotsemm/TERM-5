DECLARE
    v_new_teacher TEACHER.TEACHER%TYPE := 'АНТПВ';
    v_new_teacher_name TEACHER.TEACHER_NAME%TYPE := 'Антипов Алексей Романович';
    v_new_teacher_gender TEACHER.GENDER%TYPE := 'м';
    v_new_teacher_pulpit TEACHER.PULPIT%TYPE := 'ИСиТ';
    v_value_from_table TEACHER%ROWTYPE;
BEGIN
    INSERT INTO TEACHER(TEACHER, TEACHER_NAME, GENDER, PULPIT)
                Values(v_new_teacher, v_new_teacher_name, v_new_teacher_gender, v_new_teacher_pulpit);
    COMMIT;
    
    DELETE
        FROM TEACHER
        WHERE TEACHER = 'АНТПВ';
    
    IF SQL%ROWCOUNT = 0 THEN
        DBMS_OUTPUT.PUT_LINE('Не было удалено ни одной новой строки.');
    ELSE
        DBMS_OUTPUT.PUT_LINE('Успешно удалено ' || SQL%ROWCOUNT || ' строк');
    END IF;
    ROLLBACK;
    DBMS_OUTPUT.PUT_LINE('Удаление откачено при помощи rollback.');

    
    SELECT *
        INTO v_value_from_table
        FROM TEACHER tch
        WHERE tch.TEACHER = v_new_teacher; 
    DBMS_OUTPUT.PUT_LINE('Значение из таблицы: ' || v_value_from_table.teacher_name);
  
    
     DELETE
        FROM TEACHER
        WHERE TEACHER = 'АНТПВ';

    IF SQL%ROWCOUNT = 0 THEN
        DBMS_OUTPUT.PUT_LINE('Не было удалено ни одной новой строки.');
    ELSE
        DBMS_OUTPUT.PUT_LINE('Успешно удалено ' || SQL%ROWCOUNT || ' строк');
    END IF;
    COMMIT;
    DBMS_OUTPUT.PUT_LINE('Вставка закоммичена при помоши commit.');
    
    
    
    SELECT *
        INTO v_value_from_table
        FROM TEACHER tch
        WHERE tch.TEACHER = v_new_teacher; 
    DBMS_OUTPUT.PUT_LINE('Значение из таблицы: ' || v_value_from_table.teacher_name);
 
    EXCEPTION 
        WHEN NO_DATA_FOUND THEN
        DBMS_OUTPUT.PUT_LINE('Значение не найдено в таблице TEACHER');
  
END;
/
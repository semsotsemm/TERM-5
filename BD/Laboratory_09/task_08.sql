DECLARE
    v_new_teacher TEACHER.TEACHER%TYPE := 'АНТПВ';
    v_new_teacher_name TEACHER.TEACHER_NAME%TYPE := 'Антипов Алексей Романович';
    v_new_teacher_gender TEACHER.GENDER%TYPE := 'FFF';
    v_new_teacher_pulpit TEACHER.PULPIT%TYPE := 'ИСиТ';
BEGIN
    INSERT INTO TEACHER(TEACHER, TEACHER_NAME, GENDER, PULPIT)
                Values(v_new_teacher, v_new_teacher_name, v_new_teacher_gender, v_new_teacher_pulpit);
    EXCEPTION
        WHEN OTHERS THEN
            DBMS_OUTPUT.PUT_LINE(SQLCODE || ': Ошибка вставки значения: нарушение целостности.');
END;
/    
    
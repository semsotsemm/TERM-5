DECLARE
    v_table_teacher_name TEACHER.TEACHER_NAME%TYPE := 'Антипов Алексей Романович';
    v_teacher_fio STRING(200);
    CURSOR c_teachers IS 
        SELECT tch.TEACHER_NAME
        FROM TEACHER tch;
BEGIN
    OPEN c_teachers;
    
     LOOP
        FETCH c_teachers INTO v_table_teacher_name;
        EXIT WHEN c_teachers%NOTFOUND;
        v_teacher_fio := 
            REGEXP_SUBSTR(v_table_teacher_name, '[^ ]+', 1, 1) || ' ' || -- Фамилия
            SUBSTR(REGEXP_SUBSTR(v_table_teacher_name, '[^ ]+', 1, 2), 1, 1) || '.' || -- И.
            SUBSTR(REGEXP_SUBSTR(v_table_teacher_name, '[^ ]+', 1, 3), 1, 1) || '.';   -- О.

        DBMS_OUTPUT.PUT_LINE(v_teacher_fio);
    END LOOP;        
    
    CLOSE c_teachers;
END;
/
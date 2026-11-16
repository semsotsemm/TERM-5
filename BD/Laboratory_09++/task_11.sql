DECLARE
    CURSOR c_teacher IS
        SELECT *
            FROM TEACHER;
    v_teacher TEACHER.TEACHER%TYPE;
    v_teacher_name TEACHER.TEACHER_NAME%TYPE;
    v_teacher_gender TEACHER.GENDER%TYPE;
    v_teacher_pulpit TEACHER.PULPIT%TYPE;
BEGIN
    OPEN c_teacher;
    
    LOOP 
        FETCH c_teacher INTO v_teacher, v_teacher_name, v_teacher_gender, v_teacher_pulpit;
        EXIT WHEN c_teacher%NOTFOUND;
        DBMS_OUTPUT.PUT_LINE('Краткое имя: ' || v_teacher || ', фио: ' || v_teacher_name || ', пол: ' || v_teacher_gender || ', кафедра: ' || v_teacher_pulpit);
    END LOOP;
    
    CLOSE c_teacher;    
END;
/
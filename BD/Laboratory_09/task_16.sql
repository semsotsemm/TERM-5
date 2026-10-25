DECLARE 
    CURSOR c_hierarchy IS
        SELECT pulp.PULPIT_NAME,
                CURSOR (
                    SELECT tch.TEACHER, tch.TEACHER_NAME, tch.GENDER
                        FROM TEACHER tch 
                        WHERE tch.PULPIT = pulp.PULPIT
                )
            FROM PULPIT pulp;
            
    v_pulpit_name PULPIT.PULPIT_NAME%TYPE;
    v_teacher TEACHER.TEACHER%TYPE;
    v_teacher_name TEACHER.TEACHER_NAME%TYPE;
    v_teacher_gender TEACHER.GENDER%TYPE;
    
    c_teacher SYS_REFCURSOR;
BEGIN
    OPEN c_hierarchy;
    LOOP 
        FETCH c_hierarchy INTO v_pulpit_name, c_teacher;
        EXIT WHEN c_hierarchy%NOTFOUND;
        DBMS_OUTPUT.PUT_LINE('Кафедра ' || v_pulpit_name || ':');

        LOOP
            FETCH c_teacher INTO v_teacher, v_teacher_name, v_teacher_gender;
            EXIT WHEN c_teacher%NOTFOUND;
            DBMS_OUTPUT.PUT_LINE('Сокращение преподавателя: ' || v_teacher || ', преподаватель: ' || v_teacher_name || ', пол: ' || v_teacher_gender);
        END LOOP;
    END LOOP;
END;
/
DECLARE
    v_counter NUMBER := 1;

    v_name TEACHER.TEACHER_NAME%TYPE;
    v_pulpit TEACHER.PULPIT%TYPE;
    v_gender TEACHER.GENDER%TYPE;

    CURSOR c_teachers IS 
        SELECT tch.TEACHER_NAME, tch.GENDER, tch.PULPIT
        FROM TEACHER tch;
BEGIN
    OPEN c_teachers;
    
    LOOP
        FETCH c_teachers INTO v_name, v_gender, v_pulpit;
        EXIT WHEN c_teachers%NOTFOUND;
        DBMS_OUTPUT.PUT_LINE('Преподаватель ' || v_name || ', пол: ' || v_gender || ', кафедра: ' || v_pulpit);
        IF v_counter > 2 THEN
            DBMS_OUTPUT.PUT_LINE('-------------------------------------------------------');
            v_counter := 0;
        END IF;
        v_counter :=  v_counter + 1;
    END LOOP;
    
    CLOSE c_teachers;
END;
/
    
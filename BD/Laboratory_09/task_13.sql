DECLARE 
    CURSOR c_detail_teacher IS 
        SELECT tch.TEACHER_NAME, plp.PULPIT, plp.PULPIT_NAME, plp.FACULTY
            FROM PULPIT plp
            JOIN TEACHER tch
            ON tch.PULPIT = plp.PULPIT;
            
    v_teacher_name TEACHER.TEACHER_NAME%TYPE;
    v_pulpit PULPIT.PULPIT%TYPE;
    v_pulpit_name PULPIT.PULPIT_NAME%TYPE;
    v_pulpit_faculty PULPIT.FACULTY%TYPE;
BEGIN
    OPEN c_detail_teacher;
    LOOP
        FETCH c_detail_teacher INTO v_teacher_name, v_pulpit, v_pulpit_name, v_pulpit_faculty;
        EXIT WHEN c_detail_teacher%NOTFOUND; 
        DBMS_OUTPUT.PUT_LINE('Преподаватель: ' || v_teacher_name || ', сокращение кафедры: ' || v_pulpit || ', кафедра: ' || v_pulpit_name || ', факультет: ' || v_pulpit_faculty);
    END LOOP;
    CLOSE c_detail_teacher;
END;
/
DECLARE 
    CURSOR c_subjects IS 
        SELECT *
            FROM SUBJECT;
    v_subject SUBJECT.SUBJECT%TYPE;
    v_subject_name SUBJECT.SUBJECT_NAME%TYPE;
    v_subject_pulpit SUBJECT.PULPIT%TYPE;
BEGIN
    OPEN c_subjects;
    FETCH c_subjects INTO v_subject, v_subject_name, v_subject_pulpit;
    WHILE c_subjects%FOUND LOOP
        DBMS_OUTPUT.PUT_LINE('Предмет: ' || v_subject_name || ', сокращенное название: ' || v_subject || ', кафедра: ' || v_subject_pulpit);
        FETCH c_subjects INTO v_subject, v_subject_name, v_subject_pulpit;
    END LOOP;
    CLOSE c_subjects;
END;
/
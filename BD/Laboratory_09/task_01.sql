DECLARE
    teacher_information TEACHER%ROWTYPE;
BEGIN
    SELECT *
        INTO teacher_information
        FROM TEACHER tch
        WHERE tch.TEACHER = 'СМЛВ';
    DBMS_OUTPUT.PUT_LINE('ФИО преподавателя: ' || teacher_information.TEACHER_NAME);
    
    EXCEPTION 
        WHEN OTHERS THEN
            DBMS_OUTPUT.PUT_LINE('Преподавателя нет в таблице TEACHER');
END;
/
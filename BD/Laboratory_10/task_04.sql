DECLARE
    teacher_information TEACHER%ROWTYPE;
BEGIN
    SELECT *
        INTO teacher_information
        FROM TEACHER tch
        WHERE tch.TEACHER = 'СМЛВ2';
    DBMS_OUTPUT.PUT_LINE('ФИО преподавателя: ' || teacher_information.TEACHER_NAME);
    
    EXCEPTION 
        WHEN NO_DATA_FOUND THEN
            DBMS_OUTPUT.PUT_LINE(SQLCODE || ': Преподавателя не найдено в таблице TEACHER.');
        WHEN OTHERS THEN
            DBMS_OUTPUT.PUT_LINE(SQLCODE || ': ' || SQLERRM);
END;
/


BEGIN
    DELETE FROM TEACHER tch
        WHERE tch.TEACHER = 'СМЛВ2';
    IF SQL%NOTFOUND THEN
        DBMS_OUTPUT.PUT_LINE(SQLCODE || ': Заданного преподавателя не найдено в таблице TEACHER.');
    ELSE 
        DBMS_OUTPUT.PUT_LINE('Заданный преподаватель успешно удален из таблицы TEACHER.');
    END IF;
END;
/
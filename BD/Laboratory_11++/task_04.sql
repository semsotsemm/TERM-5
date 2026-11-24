DECLARE
    v_count NUMBER;

    FUNCTION GET_NUM_TEACHERS (FCODE FACULTY.FACULTY%TYPE) 
    RETURN NUMBER 
    IS
        v_num NUMBER;
    BEGIN
        SELECT COUNT(*)
            INTO v_num
            FROM TEACHER t
            JOIN PULPIT p ON t.PULPIT = p.PULPIT
            WHERE p.FACULTY = FCODE;
        RETURN v_num;
    END GET_NUM_TEACHERS;
BEGIN
    v_count := GET_NUM_TEACHERS('ИДиП');
    DBMS_OUTPUT.PUT_LINE('Количество преподавателей на факультете: ' || v_count);
END;
/


DECLARE
    v_count NUMBER;
    
    FUNCTION GET_NUM_SUBJECTS (PCODE SUBJECT.PULPIT%TYPE) 
    RETURN NUMBER 
    IS
        v_num NUMBER;
    BEGIN
        SELECT COUNT(*)
            INTO v_num
            FROM SUBJECT
            WHERE PULPIT = PCODE;

        RETURN v_num;
    END GET_NUM_SUBJECTS;
BEGIN
    v_count := GET_NUM_SUBJECTS('ИСиТ');
    DBMS_OUTPUT.PUT_LINE('Количество дисциплин на кафедре: ' || v_count);
END;
/
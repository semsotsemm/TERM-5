DECLARE
    v_count NUMBER;

    FUNCTION GET_NUM_TEACHERS (PCODE TEACHER.PULPIT%TYPE) RETURN NUMBER IS
        v_num NUMBER;
    BEGIN
        SELECT COUNT(*)
        INTO v_num
        FROM TEACHER
        WHERE PULPIT = PCODE;

        RETURN v_num;
    END GET_NUM_TEACHERS;
BEGIN
    v_count := GET_NUM_TEACHERS('ИСиТ');
    DBMS_OUTPUT.PUT_LINE('Количество преподавателей на кафедре: ' || v_count);
END;
/
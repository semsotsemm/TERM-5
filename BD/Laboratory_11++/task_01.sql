DECLARE
    PROCEDURE GET_TEACHERS (PCODE TEACHER.PULPIT%TYPE)
    IS
        v_found BOOLEAN := FALSE;
    BEGIN
        FOR rec IN (
            SELECT TEACHER, TEACHER_NAME, PULPIT
                FROM TEACHER
                WHERE PULPIT = PCODE
        ) LOOP
            DBMS_OUTPUT.PUT_LINE(
                TRIM(rec.TEACHER) || ' | ' || TRIM(rec.TEACHER_NAME) || ' | ' || TRIM(rec.PULPIT)
            );
            v_found := TRUE;
        END LOOP;

        IF NOT v_found THEN
            DBMS_OUTPUT.PUT_LINE('Преподаватели на кафедре ' || TRIM(PCODE) || ' не найдены.');
        END IF;
    END GET_TEACHERS;


BEGIN
    GET_TEACHERS('ИСиТ');
END;
/
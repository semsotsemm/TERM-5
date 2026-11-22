CREATE OR REPLACE PACKAGE TEACHERS AS
    PROCEDURE GET_TEACHERS (FCODE FACULTY.FACULTY%TYPE);
    PROCEDURE GET_SUBJECTS (PCODE SUBJECT.PULPIT%TYPE);
    FUNCTION GET_NUM_TEACHERS (FCODE FACULTY.FACULTY%TYPE) RETURN NUMBER;
    FUNCTION GET_NUM_SUBJECTS (PCODE SUBJECT.PULPIT%TYPE) RETURN NUMBER;
END TEACHERS;
/

CREATE OR REPLACE PACKAGE BODY TEACHERS AS

    PROCEDURE GET_TEACHERS (FCODE FACULTY.FACULTY%TYPE) IS
        v_found BOOLEAN := FALSE;
    BEGIN
        FOR rec IN (
            SELECT t.TEACHER, t.TEACHER_NAME, t.PULPIT
            FROM TEACHER t
            JOIN PULPIT p ON t.PULPIT = p.PULPIT
            WHERE p.FACULTY = FCODE
        ) LOOP
            DBMS_OUTPUT.PUT_LINE(
                TRIM(rec.TEACHER) || ' | ' || TRIM(rec.TEACHER_NAME) || ' | ' || TRIM(rec.PULPIT)
            );
            v_found := TRUE;
        END LOOP;

        IF NOT v_found THEN
            DBMS_OUTPUT.PUT_LINE('Преподаватели на факультете ' || TRIM(FCODE) || ' не найдены.');
        END IF;
    END GET_TEACHERS;

    PROCEDURE GET_SUBJECTS (PCODE SUBJECT.PULPIT%TYPE) IS
        v_found BOOLEAN := FALSE;
    BEGIN
        FOR rec IN (
            SELECT SUBJECT, SUBJECT_NAME, PULPIT
            FROM SUBJECT
            WHERE PULPIT = PCODE
        ) LOOP
            DBMS_OUTPUT.PUT_LINE(
                TRIM(rec.SUBJECT) || ' | ' || TRIM(rec.SUBJECT_NAME) || ' | ' || TRIM(rec.PULPIT)
            );
            v_found := TRUE;
        END LOOP;

        IF NOT v_found THEN
            DBMS_OUTPUT.PUT_LINE('Дисциплины на кафедре ' || TRIM(PCODE) || ' не найдены.');
        END IF;
    END GET_SUBJECTS;

    FUNCTION GET_NUM_TEACHERS (FCODE FACULTY.FACULTY%TYPE) RETURN NUMBER IS
        v_num NUMBER;
    BEGIN
        SELECT COUNT(*)
        INTO v_num
        FROM TEACHER t
        JOIN PULPIT p ON t.PULPIT = p.PULPIT
        WHERE p.FACULTY = FCODE;

        RETURN v_num;
    END GET_NUM_TEACHERS;

    FUNCTION GET_NUM_SUBJECTS (PCODE SUBJECT.PULPIT%TYPE) RETURN NUMBER IS
        v_num NUMBER;
    BEGIN
        SELECT COUNT(*)
        INTO v_num
        FROM SUBJECT
        WHERE PULPIT = PCODE;

        RETURN v_num;
    END GET_NUM_SUBJECTS;

END TEACHERS;
/

SELECT OBJECT_NAME, OBJECT_TYPE, STATUS
    FROM USER_OBJECTS
    WHERE OBJECT_NAME = 'TEACHERS';


BEGIN
    DBMS_OUTPUT.PUT_LINE('--- Преподаватели факультета ---');
    TEACHERS.GET_TEACHERS('ИДиП');

    DBMS_OUTPUT.PUT_LINE('--- Дисциплины кафедры ---');
    TEACHERS.GET_SUBJECTS('ИСиТ');

    DBMS_OUTPUT.PUT_LINE('--- Количество ---');
    DBMS_OUTPUT.PUT_LINE('Преподавателей на факультете: ' || TEACHERS.GET_NUM_TEACHERS('ИДиП'));
    DBMS_OUTPUT.PUT_LINE('Дисциплин на кафедре: ' || TEACHERS.GET_NUM_SUBJECTS('ИСиТ'));
END;
/
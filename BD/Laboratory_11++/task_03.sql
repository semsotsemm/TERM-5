CREATE OR REPLACE PROCEDURE GET_TEACHERS (FCODE FACULTY.FACULTY%TYPE) 
IS
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
/


CREATE OR REPLACE PROCEDURE GET_SUBJECTS (PCODE SUBJECT.PULPIT%TYPE) 
IS
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
/


DECLARE
    v_fcode FACULTY.FACULTY%TYPE := 'ХТиТ'; 
    v_pcode SUBJECT.PULPIT%TYPE  := 'ИСиТ'; 
BEGIN
    DBMS_OUTPUT.PUT_LINE('=== Преподаватели факультета ' || v_fcode || ' ===');
    GET_TEACHERS(v_fcode);

    DBMS_OUTPUT.PUT_LINE('');
    DBMS_OUTPUT.PUT_LINE('=== Дисциплины кафедры ' || v_pcode || ' ===');
    GET_SUBJECTS(v_pcode);
END;
/
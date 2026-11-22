SET SERVEROUTPUT ON;

DECLARE
    v_faculty      FACULTY.FACULTY%TYPE := 'ИДиП';
    v_pulpit       SUBJECT.PULPIT%TYPE  := 'ИСиТ';
    v_teachers_cnt NUMBER;
    v_subjects_cnt NUMBER;
BEGIN
    DBMS_OUTPUT.PUT_LINE('=== 1. Преподаватели факультета ' || TRIM(v_faculty) || ' ===');
    TEACHERS.GET_TEACHERS(v_faculty);

    DBMS_OUTPUT.PUT_LINE('');
    DBMS_OUTPUT.PUT_LINE('=== 2. Дисциплины кафедры ' || TRIM(v_pulpit) || ' ===');
    TEACHERS.GET_SUBJECTS(v_pulpit);

    DBMS_OUTPUT.PUT_LINE('');
    DBMS_OUTPUT.PUT_LINE('=== 3. Количество преподавателей на факультете ===');
    v_teachers_cnt := TEACHERS.GET_NUM_TEACHERS(v_faculty);
    DBMS_OUTPUT.PUT_LINE('Факультет ' || TRIM(v_faculty) || ': ' || v_teachers_cnt);

    DBMS_OUTPUT.PUT_LINE('');
    DBMS_OUTPUT.PUT_LINE('=== 4. Количество дисциплин на кафедре ===');
    v_subjects_cnt := TEACHERS.GET_NUM_SUBJECTS(v_pulpit);
    DBMS_OUTPUT.PUT_LINE('Кафедра ' || TRIM(v_pulpit) || ': ' || v_subjects_cnt);
END;
/
DECLARE 
    v_cursor SYS_REFCURSOR;
    v_min_capacity NUMBER := 20;
    v_max_capacity NUMBER := 40;
    
    v_auditorium_name AUDITORIUM.AUDITORIUM_NAME%TYPE;
    v_auditorium_capacity AUDITORIUM.AUDITORIUM_CAPACITY%TYPE;
    v_auditorium_type AUDITORIUM.AUDITORIUM_TYPE%TYPE;
BEGIN
    OPEN v_cursor FOR 
        SELECT aud.AUDITORIUM_NAME, aud.AUDITORIUM_CAPACITY, aud.AUDITORIUM_TYPE
            FROM AUDITORIUM aud
            WHERE aud.AUDITORIUM_CAPACITY BETWEEN v_min_capacity AND v_max_capacity;
    
    LOOP
        FETCH v_cursor INTO v_auditorium_name, v_auditorium_capacity, v_auditorium_type;
        EXIT WHEN v_cursor%NOTFOUND;
        DBMS_OUTPUT.PUT_LINE('Аудитория: ' || v_auditorium_name || ', тип: ' || v_auditorium_type || ', вместимость: ' || v_auditorium_capacity);
    END LOOP;
    
    CLOSE v_cursor;

END;
/
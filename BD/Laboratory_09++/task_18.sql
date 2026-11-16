DECLARE
    CURSOR c_auditorium(v_min_capacity NUMBER, v_max_capacity NUMBER) IS 
        SELECT aud.AUDITORIUM_NAME, aud.AUDITORIUM_CAPACITY, aud.AUDITORIUM_TYPE
            FROM AUDITORIUM aud
            WHERE aud.AUDITORIUM_CAPACITY BETWEEN v_min_capacity AND v_max_capacity
            FOR UPDATE;
    
    v_name AUDITORIUM.AUDITORIUM_NAME%TYPE;
    v_capacity AUDITORIUM.AUDITORIUM_CAPACITY%TYPE;
    v_type AUDITORIUM.AUDITORIUM_TYPE%TYPE;
BEGIN
    DBMS_OUTPUT.PUT_LINE('Список аудиторий до удаления: ');
    OPEN c_auditorium(0, 20);
    LOOP
        FETCH c_auditorium INTO v_name, v_capacity, v_type;
        EXIT WHEN c_auditorium%NOTFOUND;
        DBMS_OUTPUT.PUT_LINE('Аудитория: ' || v_name || ', тип аудитории: ' || v_type || ', вместимость: ' || v_capacity);
    END LOOP;
    CLOSE c_auditorium;
    COMMIT;

    
    OPEN c_auditorium(0, 20);
    LOOP
        FETCH c_auditorium INTO v_name, v_capacity, v_type;
        EXIT WHEN c_auditorium%NOTFOUND;
        
        DELETE AUDITORIUM
        WHERE CURRENT OF c_auditorium;
    END LOOP;
    CLOSE c_auditorium;


    DBMS_OUTPUT.PUT_LINE('Список аудиторий после обновления обновления: ');    
    OPEN c_auditorium(0, 20);
    LOOP
        FETCH c_auditorium INTO v_name, v_capacity, v_type;
        EXIT WHEN c_auditorium%NOTFOUND;
        DBMS_OUTPUT.PUT_LINE('Аудитория: ' || v_name || ', тип аудитории: ' || v_type || ', вместимость: ' || v_capacity);
    END LOOP;
    CLOSE c_auditorium;
    
    ROLLBACK;
    DBMS_OUTPUT.PUT_LINE('Таблица возвращена в исходное состояние.');
END;
/
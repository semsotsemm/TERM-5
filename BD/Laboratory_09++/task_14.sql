DECLARE
    v_auditotium_type AUDITORIUM.AUDITORIUM_TYPE%TYPE;
    v_auditotium_capacity AUDITORIUM.AUDITORIUM_CAPACITY%TYPE;
    v_auditotium_name AUDITORIUM.AUDITORIUM_NAME%TYPE;

    CURSOR c_auditorium (min_capacity NUMBER, max_capacity NUMBER) IS 
        SELECT aud.AUDITORIUM_TYPE, aud.AUDITORIUM_CAPACITY, aud.AUDITORIUM_NAME
            FROM AUDITORIUM aud
            WHERE aud.AUDITORIUM_CAPACITY < max_capacity AND aud.AUDITORIUM_CAPACITY > min_capacity;
BEGIN
    DBMS_OUTPUT.PUT_LINE('Аудитории с вместимостью менее 20:');
    FOR auditorium IN c_auditorium(0,  20) LOOP
        DBMS_OUTPUT.PUT_LINE('Аудитория ' || auditorium.AUDITORIUM_NAME || ': тип: ' || auditorium.AUDITORIUM_TYPE || ', вместимость: ' ||auditorium.AUDITORIUM_CAPACITY);
    END LOOP;
    
    
    DBMS_OUTPUT.PUT_LINE('Аудитории с вместимостью от 21 до 30:');
    OPEN c_auditorium(21, 30);
    
    LOOP 
        FETCH c_auditorium INTO v_auditotium_type, v_auditotium_capacity, v_auditotium_name;
        EXIT WHEN c_auditorium%NOTFOUND;
        DBMS_OUTPUT.PUT_LINE('Аудитория ' || v_auditotium_name || ': тип: ' || v_auditotium_type || ', вместимость: ' ||v_auditotium_capacity);
    END LOOP;
        
    CLOSE c_auditorium;
    
    
    DBMS_OUTPUT.PUT_LINE('Аудитории с вместимостью от 31 до 60:');
    OPEN c_auditorium(31, 60);
    
    FETCH c_auditorium INTO v_auditotium_type, v_auditotium_capacity, v_auditotium_name;
    WHILE c_auditorium%FOUND LOOP 
        EXIT WHEN c_auditorium%NOTFOUND;
        DBMS_OUTPUT.PUT_LINE('Аудитория ' || v_auditotium_name || ': тип: ' || v_auditotium_type || ', вместимость: ' ||v_auditotium_capacity);
        FETCH c_auditorium INTO v_auditotium_type, v_auditotium_capacity, v_auditotium_name;
    END LOOP;
        
    CLOSE c_auditorium;
    
    DBMS_OUTPUT.PUT_LINE('Аудитории с вместимостью от 61 до 80:');
     FOR auditorium IN c_auditorium(61,  80) LOOP
        DBMS_OUTPUT.PUT_LINE('Аудитория ' || auditorium.AUDITORIUM_NAME || ': тип: ' || auditorium.AUDITORIUM_TYPE || ', вместимость: ' ||auditorium.AUDITORIUM_CAPACITY);
    END LOOP;
    
    DBMS_OUTPUT.PUT_LINE('Аудитории с вместимостью более 81:');
    FOR auditorium IN c_auditorium(81,  10000) LOOP
        DBMS_OUTPUT.PUT_LINE('Аудитория ' || auditorium.AUDITORIUM_NAME || ': тип: ' || auditorium.AUDITORIUM_TYPE || ', вместимость: ' ||auditorium.AUDITORIUM_CAPACITY);
    END LOOP;
    
END;
/
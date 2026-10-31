SET SERVEROUTPUT ON;

DECLARE
    CURSOR c_jubilee IS
        SELECT 
            TEACHER_NAME, 
            BIRTHDAY,
            (EXTRACT(YEAR FROM SYSDATE) + 1) - EXTRACT(YEAR FROM BIRTHDAY) AS AGE_NEXT_YEAR
        FROM 
            TEACHER
        WHERE 
            -- Разница между следующим годом и годом рождения должна делиться на 5 без остатка
            MOD((EXTRACT(YEAR FROM SYSDATE) + 1) - EXTRACT(YEAR FROM BIRTHDAY), 5) = 0
            AND BIRTHDAY IS NOT NULL;
BEGIN
    DBMS_OUTPUT.PUT_LINE('--- Юбиляры следующего года ---');
    
    FOR v_rec IN c_jubilee LOOP
        DBMS_OUTPUT.PUT_LINE(
            v_rec.TEACHER_NAME || 
            ' (Дата рождения: ' || TO_CHAR(v_rec.BIRTHDAY, 'DD.MM.YYYY') || ') ' ||
            '— исполняется ' || v_rec.AGE_NEXT_YEAR || ' лет.'
        );
    END LOOP;
END;
/
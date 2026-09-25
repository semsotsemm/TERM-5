-- Заполнить данными таблицу --
BEGIN
    FOR i IN 1..10000 LOOP
        INSERT INTO AAR_T1 (id, description) 
        VALUES (i + 10, 'Сгенерированная строка ' || i);
    END LOOP;
    
    COMMIT;
END;
/

SELECT *
    FROM AAR_T1;
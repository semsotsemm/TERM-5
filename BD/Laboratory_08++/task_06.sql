DECLARE 
    CURSOR c_spec_chars IS 
        SELECT keyword
            FROM v$reserved_words
            WHERE LENGTH(keyword) = 1
            AND REGEXP_LIKE(keyword, '[^[:alnum:][:space:]]');
BEGIN
    FOR r IN c_spec_chars LOOP
        DBMS_OUTPUT.PUT_LINE('Спецсимвол: ' || r.keyword);
    END LOOP;
END;
/
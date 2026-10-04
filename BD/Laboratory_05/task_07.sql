-- Создать таблицу, которая помещается в KEEP -- 
CREATE TABLE keep_test_table (
    id NUMBER PRIMARY KEY,
    val VARCHAR2(50)
)
    SEGMENT CREATION IMMEDIATE
    STORAGE (BUFFER_POOL KEEP);


SELECT segment_name AS "сегмент", segment_type AS "тип", tablespace_name AS "ts", bytes / 1024 AS "размер", buffer_pool AS "пул в SGA"
    FROM user_segments 
    WHERE segment_name = 'KEEP_TEST_TABLE';
    
DROP TABLE keep_test_table PURGE;
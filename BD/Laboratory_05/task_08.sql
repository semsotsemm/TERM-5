-- Кэшируется в default --
CREATE TABLE default_test_table (
    id NUMBER PRIMARY KEY,
    val VARCHAR2(50)
)
    STORAGE (BUFFER_POOL DEFAULT);

SELECT segment_name AS "сегмент", segment_type AS "тип", bytes / 1024 AS "размер", buffer_pool AS "пул кэширования"
    FROM user_segments 
    WHERE segment_name = 'DEFAULT_TEST_TABLE';

DROP TABLE default_test_table PURGE;
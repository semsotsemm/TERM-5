-- Удаление таблицы, просмотр  списка сегментов --
DROP TABLE AAR_T1;

SELECT segment_name, segment_type 
FROM dba_segments 
WHERE tablespace_name = 'AAR_QDATA';

SELECT object_name, original_name, type, droptime 
FROM user_recyclebin;
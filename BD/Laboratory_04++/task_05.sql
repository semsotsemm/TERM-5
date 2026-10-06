-- Восстановить удаленную таблицу --
FLASHBACK TABLE AAR_T1 TO BEFORE DROP;

SELECT segment_name, segment_type, bytes/1024 AS size_kb
FROM dba_segments
WHERE tablespace_name = 'AAR_QDATA';
-- Получить все файлы табличных простарнст --
SELECT tablespace_name, file_name, bytes/1024/1024 AS size_mb, 'DATAFILE' AS file_type 
    FROM dba_data_files
    UNION ALL
    SELECT tablespace_name, file_name, bytes/1024/1024 AS size_mb, 'TEMPFILE' AS file_type 
    FROM dba_temp_files;
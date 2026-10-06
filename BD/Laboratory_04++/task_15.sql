-- Определить номер последнего архива.  --
SELECT MAX(sequence#) AS last_archive_number 
    FROM v$archived_log;
-- Список серверных процессов -- 
SELECT 
    spid AS os_process_id, 
    program AS process_name
FROM 
    v$process
WHERE 
    background IS NULL 
    AND spid IS NOT NULL;
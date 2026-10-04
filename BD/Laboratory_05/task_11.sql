-- Режимы текущего соединения с инстансом --
SELECT 
    username, 
    sid, 
    serial#, 
    server AS connection_mode
FROM 
    v$session
WHERE 
    username IS NOT NULL;
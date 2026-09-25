-- Получить перечень групп журналов повтора, определить текущую группу --
SELECT group#, members, bytes/1024/1024 AS size_mb, status, archived 
FROM v$log 
ORDER BY group#;

SELECT group# 
FROM v$log 
WHERE status = 'CURRENT';
-- Размер буфера журнала повтора --
SELECT name AS memory_component, ROUND(bytes / 1024 / 1024, 2) AS size_mb
    FROM v$sgainfo 
        WHERE name = 'Redo Buffers';
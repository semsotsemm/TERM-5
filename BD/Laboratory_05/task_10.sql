-- Свободная память в большом пуле --
SELECT pool AS sga_pool, name AS memory_type, ROUND(bytes / 1024 / 1024, 2) AS free_mb
    FROM v$sgastat 
    WHERE pool = 'large pool' AND name = 'free memory';
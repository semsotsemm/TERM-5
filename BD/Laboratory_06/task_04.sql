-- Определить объем доступной свободной памяти SGA -- 
SELECT NVL(pool, 'UNALLOCATED') AS sga_pool,  name AS memory_type, ROUND(bytes / 1024 / 1024, 2) AS free_mb 
    FROM v$sgastat 
    WHERE name = 'free memory';
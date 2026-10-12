-- Размер гранулы для каждого пула -- 
SELECT  component AS sga_pool, granule_size / 1024 / 1024 AS granule_size_mb 
    FROM v$sga_dynamic_components;
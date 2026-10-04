-- Размер основых пулов SGA -- 
SELECT name AS sga_component,  ROUND(bytes / 1024 / 1024, 2) AS size_mb
    FROM v$sgainfo;
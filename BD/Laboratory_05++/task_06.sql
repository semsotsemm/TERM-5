-- Определить размеры пулов KEEP, DEFAULT, RECYCLE мезанищм стек --
SELECT component AS buffer_pool_type, ROUND(current_size / 1024 / 1024, 2) AS current_size_mb
    FROM v$sga_dynamic_components
    WHERE component IN ('DEFAULT buffer cache', 'KEEP buffer cache', 'RECYCLE buffer cache');
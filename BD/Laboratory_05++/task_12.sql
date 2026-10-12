-- Работающие фоновые процессы -- 
SELECT 
    name AS process_name, 
    description 
FROM 
    v$bgprocess 
WHERE 
    paddr <> HEXTORAW('00');
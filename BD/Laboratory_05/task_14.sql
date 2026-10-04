-- Работающие DBWs -- 
SELECT 
    COUNT(*) AS dbwn_count
FROM 
    v$bgprocess 
WHERE 
    name LIKE 'DBW%' 
    AND paddr <> HEXTORAW('00');
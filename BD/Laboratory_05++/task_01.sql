-- Общий размер области SGA --
SELECT SUM(value)/1024/1024 AS "TOTAL_SGA_MB" 
    FROM v$sga;
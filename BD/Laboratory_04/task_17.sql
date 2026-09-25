ALTER SYSTEM SWITCH LOGFILE;

SELECT sequence#, name, first_change#, next_change# 
    FROM v$archived_log;

SELECT sequence#, status, first_change#, next_change# 
    FROM v$log;
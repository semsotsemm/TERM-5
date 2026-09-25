-- Удалить ранее созданную  группу журнала повтора --
ALTER SESSION SET CONTAINER = CDB$ROOT;
ALTER SYSTEM CHECKPOINT;
SELECT group#, status FROM v$log;
    
ALTER DATABASE DROP LOGFILE GROUP 3;
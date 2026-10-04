-- Создать группу журнала повтора --
ALTER SESSION SET CONTAINER = CDB$ROOT;

SHOW CON_NAME;

ALTER DATABASE ADD LOGFILE (
    '/opt/oracle/oradata/FREE/redo_ext_a.log',
    '/opt/oracle/oradata/FREE/redo_ext_b.log',
    '/opt/oracle/oradata/FREE/redo_ext_c.log'
) SIZE 50M;

SELECT group#, members, status 
    FROM v$log;

SELECT group#, type, member 
    FROM v$logfile;

ALTER SYSTEM SWITCH LOGFILE;
SELECT group#, status,  first_change#, next_change#
    FROM v$log;
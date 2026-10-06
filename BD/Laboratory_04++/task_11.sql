-- Пройти полный цикл переключений, записать серверное время в моментт первого подключения --
SELECT TO_CHAR(SYSDATE, 'DD-MM-YYYY HH24:MI:SS') AS server_time FROM dual;

ALTER SYSTEM SWITCH LOGFILE;
ALTER SYSTEM SWITCH LOGFILE;
ALTER SYSTEM SWITCH LOGFILE;
ALTER SYSTEM SWITCH LOGFILE;

SELECT group#, status FROM v$log ORDER BY group#;
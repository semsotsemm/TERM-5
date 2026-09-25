-- Получить перечень файлов всех журналов повтора инстанса. --
SELECT group#, status, type, member 
    FROM v$logfile;
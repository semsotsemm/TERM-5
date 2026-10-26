-- 1. Вывести имя базы данных.
SELECT name, log_mode FROM v$database;

-- 2. Показать все представления словаря.
SELECT table_name, comments FROM dictionary;

-- 3. Получить список PDB.
SELECT name, open_mode, restricted FROM v$pdbs;

-- 4. Получить список сессий.
SELECT sid, serial#, username, machine, status FROM v$session;

-- 5. Получить список общих пользователей.
SELECT username, common FROM dba_users WHERE common = 'YES';

-- 6. Вывести всех пользователей из файла паролей.
SELECT username, sysdba, sysoper FROM v$pwfile_users;

-- 7. Получить список всех общих ролей.
SELECT role, common FROM dba_roles WHERE common = 'YES';

-- 8. Получить список привилегий роли (на примере роли CONNECT или всех).
SELECT role, privilege FROM role_sys_privs;

-- 9. Получить список параметров профиля безопасности DEFAULT.
SELECT resource_name, limit FROM dba_profiles WHERE profile = 'DEFAULT';

-- 10. Получить список параметров экземпляра (инстанса).
SELECT name, value, description FROM v$parameter;

-- 11. Получить список файлов параметров.
SELECT name, value FROM v$parameter WHERE name = 'spfile';

-- 12. Получить список управляющих файлов (расположение контрольного файла).
SELECT name, status FROM v$controlfile;

-- 13. Получить список сервисов.
SELECT name, network_name FROM v$services;

-- 14. Получить список действующих фоновых процессов.
SELECT name, description, paddr FROM v$bgprocess WHERE paddr <> '00';

-- 15. Получить список работающих в настоящее время серверных процессов.
SELECT pid, spid, pname, username FROM v$process WHERE background IS NULL AND pname IS NULL;

-- 16. Вывести все группы журналов повтора.
SELECT group#, members, bytes, status FROM v$log;

-- 17. Вывести все лог-файлы (архивы) журнала повтора.
-- Файлы журналов:
SELECT group#, member, status FROM v$logfile;
-- Архивы журналов:
SELECT name, sequence#, applied FROM v$archived_log;

-- 18. Получить, ведется ли архивирование файлов журналов повтора.
SELECT log_mode FROM v$database;

-- 19. Получить список файлов табличных пространств.
SELECT tablespace_name, file_name, bytes FROM dba_data_files;

-- 20. Получить список файлов временных табличных пространств.
SELECT tablespace_name, file_name, bytes FROM dba_temp_files;

-- 21. Получить размер блока данных (двумя способами).
-- Способ 1 (через параметры):
SELECT value AS block_size FROM v$parameter WHERE name = 'db_block_size';
-- Способ 2 (через табличные пространства):
SELECT tablespace_name, block_size FROM dba_tablespaces;

-- 22. Получить список основных областей SGA и их размер.
SELECT name, bytes FROM v$sgastat;

-- 23. Получить размер свободного кэша.
SELECT pool, name, bytes FROM v$sgastat WHERE name = 'free memory';

-- 24. Получить размер гранулы.
SELECT component, granule_size FROM v$sga_dynamic_components;

-- 25. Показать, в каком пуле лежит таблица пользователя.
SELECT table_name, buffer_pool FROM user_tables;

-- 26. Получить список объектов пользователя.
SELECT object_name, object_type, status FROM user_objects;

-- 27. Получить список типов сегментов пользователя.
SELECT DISTINCT segment_type FROM user_segments;

-- 28. Получить список выделенных экстентов для конкретного пользователя (текущего).
SELECT segment_name, segment_type, extent_id, bytes, blocks FROM user_extents;

-- 29. Вывести экстенты для всех пользователей.
SELECT owner, segment_name, extent_id, bytes FROM dba_extents;

-- 30. Получить количество блоков для каждой таблицы пользователя.
SELECT table_name, blocks FROM user_tables;

-- 31. Получить размер данных в таблице пользователя.
SELECT segment_name, bytes, blocks FROM user_segments WHERE segment_type = 'TABLE';

-- 32. Получить размер данных в наибольшей таблице пользователя.
SELECT MAX(bytes) FROM user_segments WHERE segment_type = 'TABLE';

-- 33. Вывести все объекты в корзине пользователя.
SELECT object_name, original_name, type, droptime FROM user_recyclebin;

-- 34. Получить объем памяти, выделенный под объекты пользовательской корзины.
WITH rb_data AS (
    SELECT NVL(SUM(space), 0) AS total_blocks 
    FROM user_recyclebin
),
block_data AS (
    SELECT TO_NUMBER(value) AS block_size 
    FROM v$parameter 
    WHERE name = 'db_block_size'
)
SELECT 
    r.total_blocks, 
    (r.total_blocks * b.block_size) AS total_bytes 
FROM rb_data r, block_data b;
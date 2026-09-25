-- Вывести все табличные пространства, все  файлы, все роли при помощи dda --
SELECT tablespace_name, status, contents 
    FROM dba_tablespaces;

SELECT tablespace_name, file_name, bytes/1024/1024 AS size_mb, 'DATAFILE' AS file_type 
    FROM dba_data_files
    UNION ALL
    SELECT tablespace_name, file_name, bytes/1024/1024 AS size_mb, 'TEMPFILE' AS file_type 
    FROM dba_temp_files;

SELECT role, privilege 
    FROM role_sys_privs;

SELECT profile, resource_name, limit 
    FROM dba_profiles;

SELECT u.username, u.account_status, r.granted_role 
    FROM dba_users u
    LEFT JOIN dba_role_privs r 
    ON u.username = r.grantee;
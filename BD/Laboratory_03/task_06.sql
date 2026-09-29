-- Подключится к созданному (з.4) pdb, создать свои объекты --
ALTER SESSION SET CONTAINER = AAR_PDB;
SHOW CON_NAME;

CREATE TABLESPACE AAR_TS
    DATAFILE '/opt/oracle/oradata/FREE/aar_pdb/aar_ts_02.dbf' 
    SIZE 50M
    AUTOEXTEND ON NEXT 10M;
    

CREATE PROFILE AAR_PROFILE LIMIT
  PASSWORD_LIFE_TIME 180
  SESSIONS_PER_USER 5
  FAILED_LOGIN_ATTEMPTS 3
  PASSWORD_LOCK_TIME 1;

CREATE ROLE AAR_ROLE;
GRANT CREATE SESSION, CREATE TABLE, CREATE VIEW TO AAR_ROLE;

CREATE USER U1_AAR_PDB IDENTIFIED BY "12345678"
PROFILE AAR_PROFILE;

GRANT AAR_ROLE TO U1_AAR_PDB;
ALTER USER U1_AAR_PDB QUOTA UNLIMITED ON AAR_TS;


-- Проверка --
SELECT tablespace_name, status, contents 
    FROM dba_tablespaces 
    WHERE tablespace_name = 'AAR_TS';

SELECT file_name, tablespace_name, bytes/1024/1024 AS size_mb, autoextensible 
    FROM dba_data_files 
    WHERE tablespace_name = 'AAR_TS';
    
SELECT role, privilege 
    FROM role_sys_privs 
    WHERE role = 'AAR_ROLE';

SELECT username, account_status, default_tablespace, temporary_tablespace, profile 
    FROM dba_users 
    WHERE username = 'U1_AAR_PDB';


SELECT grantee, granted_role 
    FROM dba_role_privs 
    WHERE grantee = 'U1_AAR_PDB';


-- Удаление объектво -- 
DROP USER U1_AAR_PDB CASCADE;
DROP ROLE AAR_ROLE;
DROP PROFILE AAR_PROFILE CASCADE;
DROP TABLESPACE AAR_TS INCLUDING CONTENTS AND DATAFILES;
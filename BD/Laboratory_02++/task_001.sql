-- Создание объектов --
CREATE TABLESPACE AAR_TableSpace
    DATAFILE 'AAR_TS.dbf'
    SIZE 10M
    AUTOEXTEND ON NEXT 5M
    MAXSIZE 100M;


CREATE ROLE AAR_rl;


GRANT CREATE SESSION TO AAR_rl;
GRANT CREATE TABLE TO AAR_rl;
GRANT CREATE VIEW TO AAR_rl;
GRANT CREATE PROCEDURE TO AAR_rl;


CREATE PROFILE AAR_profile LIMIT
    PASSWORD_LIFE_TIME 180              
    SESSIONS_PER_USER 3                 
    FAILED_LOGIN_ATTEMPTS 5             
    PASSWORD_LOCK_TIME 1                
    PASSWORD_GRACE_TIME 7;              
    
    
CREATE USER AAR_USER IDENTIFIED BY 12345678
    DEFAULT TABLESPACE AAR_TableSpace
    QUOTA UNLIMITED ON AAR_TableSpace
    PROFILE AAR_profile;


GRANT AAR_rl TO AAR_USER;


-- Удаление объектов --
DROP USER AAR_USER CASCADE;


DROP TABLESPACE AAR_TableSpace
    INCLUDING CONTENTS
    AND DATAFILES
    CASCADE CONSTRAINTS;


DROP ROLE AAR_rl;


DROP PROFILE AAR_PROFILE CASCADE;


-- Проверка --
SELECT 
    username, 
    account_status, 
    default_tablespace, 
    temporary_tablespace, 
    profile 
    FROM dba_users 
    WHERE username = 'AAR_USER';
    
    
SELECT tablespace_name
    FROM dba_tablespaces
    WHERE tablespace_name = 'AAR_TABLESPACE';
    

SELECT role
    FROM dba_roles
    WHERE role = 'AAR_RL';
    

SELECT profile
    FROM dba_profiles
    WHERE profile = 'AAR_PROFILE';
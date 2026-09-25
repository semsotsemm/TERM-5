CREATE USER AARCORE 
    IDENTIFIED BY 12345678
    DEFAULT TABLESPACE TS_AAR     
    TEMPORARY TABLESPACE TS_AAR_TEMP
    PROFILE PF_AARCORE
    ACCOUNT UNLOCK
    PASSWORD EXPIRE;

GRANT RL_AARCORE TO AARCORE;
    

DROP USER AARCORE CASCADE;


SELECT 
    username, 
    account_status, 
    default_tablespace, 
    temporary_tablespace, 
    profile 
    FROM dba_users 
    WHERE username = 'AARCORE';
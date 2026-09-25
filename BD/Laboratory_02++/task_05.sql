SELECT 
    role, 
    password_required, 
    authentication_type
    FROM dba_roles
    WHERE role = 'RL_AARCORE';


SELECT 
    role, 
    privilege, 
    admin_option
    FROM role_sys_privs
    WHERE role = 'RL_AARCORE';
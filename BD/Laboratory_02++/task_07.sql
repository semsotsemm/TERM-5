SELECT DISTINCT profile 
    FROM dba_profiles;


SELECT resource_name, resource_type, limit 
    FROM dba_profiles 
    WHERE profile = 'PF_AARCORE';
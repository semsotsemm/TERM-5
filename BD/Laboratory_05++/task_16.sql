-- Параметры диспетчеров --
SELECT 
    name AS parameter_name, 
    value AS parameter_value
FROM 
    v$parameter 
WHERE 
    name LIKE '%dispatcher%';
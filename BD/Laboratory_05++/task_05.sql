-- Максимальный и целевой размер SGA --
SELECT name AS parameter_name, display_value AS configured_size
    FROM  v$parameter 
    WHERE name IN ('sga_max_size', 'sga_target');
-- Вывести все PDB, их состояния -- 
SELECT con_id, name, open_mode, restricted, open_time
    FROM   v$pdbs;
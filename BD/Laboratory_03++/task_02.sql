-- Вывести перечень экземпляров -- 
SELECT inst_id, instance_name, host_name, version, startup_time, status, database_status
    FROM gv$instance;

-- Удалить табличное пространство и его файл --
DROP TABLESPACE AAR_QDATA INCLUDING CONTENTS AND DATAFILES;

SELECT tablespace_name 
    FROM dba_tablespaces 
    WHERE tablespace_name = 'AAR_QDATA';
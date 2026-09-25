SELECT 
    ts.TABLESPACE_NAME, ts.MAX_SIZE, ts.STATUS, ts.CONTENTS, ts.LOGGING, df.FILE_NAME
    FROM dba_tablespaces ts
    join dba_data_files df
    on ts.TABLESPACE_NAME = df.TABLESPACE_NAME;
-- Создать табличное простарнство и таблицу в нем --
CREATE TABLESPACE AAR_QDATA
DATAFILE '/opt/oracle/oradata/FREE/aar_pdb/aar_qdata_01.dbf' SIZE 10M
OFFLINE;

ALTER TABLESPACE AAR_QDATA ONLINE;

ALTER USER U1_AAR_PDB QUOTA 2M ON AAR_QDATA;

CREATE TABLE AAR_T1 (
    id NUMBER PRIMARY KEY,
    description VARCHAR2(50)
) TABLESPACE AAR_QDATA;

INSERT INTO AAR_T1 (id, description) VALUES (1, 'Строка 1');
INSERT INTO AAR_T1 (id, description) VALUES (2, 'Строка 2');
INSERT INTO AAR_T1 (id, description) VALUES (3, 'Строка 3');
COMMIT;
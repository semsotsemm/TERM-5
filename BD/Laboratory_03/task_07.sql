-- Подключится  к пользователю PDB, создать таблицу, вставить значения. -- 

ALTER SESSION SET CONTAINER = AAR_PDB;
SHOW CON_NAME;

CREATE TABLE AAR_table (
    id NUMBER PRIMARY KEY,
    item_name VARCHAR2(50),
    quantity NUMBER
);

INSERT INTO AAR_table (id, item_name, quantity) VALUES (1, 'Ноутбук', 5);
INSERT INTO AAR_table (id, item_name, quantity) VALUES (2, 'Монитор', 12);
INSERT INTO AAR_table (id, item_name, quantity) VALUES (3, 'Клавиатура', 30);

COMMIT;

SELECT username, default_tablespace, temporary_tablespace 
    FROM user_users;

SELECT * 
    FROM AAR_table;

DROP TABLE AAR_table PURGE;
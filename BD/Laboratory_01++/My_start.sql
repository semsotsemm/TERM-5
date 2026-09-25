-- 9 --
CREATE TABLE AAR_t (
    x NUMBER(3),
    s VARCHAR2(50)
);


-- 11--
INSERT INTO AAR_t (x, s) VALUES (1, 'Пользователь 1');
INSERT INTO AAR_t (x, s) VALUES (2, 'Пользователь 2');
INSERT INTO AAR_t (x, s) VALUES (3, 'Пользователь 3');
COMMIT;


-- 12 --
UPDATE AAR_t
    SET x = x * 2
    WHERE x = 1 or 2;

COMMIT;


-- 13 --
SELECT x, s 
    FROM AAR_t 
    WHERE x > 3;

SELECT 
    COUNT(x) AS count_rows,
    SUM(x) AS sum_x, 
    MAX(x) AS max_x 
    FROM AAR_t;


-- 14 --
DELETE FROM AAR_t 
    WHERE x = 6;

COMMIT;


-- 15 --
ALTER TABLE AAR_t 
    ADD CONSTRAINT pk_AAR_t PRIMARY KEY (x);

CREATE TABLE AAR_t1 (
    id NUMBER(3),
    x_ref NUMBER(3),
    info VARCHAR2(50),
    CONSTRAINT fk_AAR_t1 FOREIGN KEY (x_ref) REFERENCES AAR_t(x)
);

INSERT INTO AAR_t1 (id, x_ref, info) VALUES (1, 2, '1');
INSERT INTO AAR_t1 (id, x_ref, info) VALUES (2, 4, '2');
INSERT INTO AAR_t1 (id, x_ref, info) VALUES (3, NULL, '3');
COMMIT;


-- 16 --
SELECT 
    a.x, a.s, b.id, b.info
    FROM AAR_t a
    INNER JOIN AAR_t1 b ON a.x = b.x_ref;

SELECT a.x, a.s, b.id, b.info
    FROM AAR_t a
    LEFT JOIN AAR_t1 b ON a.x = b.x_ref;

SELECT a.x, a.s, b.id, b.info
    FROM AAR_t a
    RIGHT JOIN AAR_t1 b ON a.x = b.x_ref;


-- 18 --
DROP TABLE AAR_t1 PURGE;
DROP TABLE AAR_t PURGE;
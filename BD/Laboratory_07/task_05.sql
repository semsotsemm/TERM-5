-- Получить значение меньше минимального --
CREATE SEQUENCE S3
    START WITH 10
    INCREMENT BY -10
    MINVALUE -100
    MAXVALUE 100  
    NOCYCLE
    ORDER;
    
SELECT S3.NEXTVAL
    FROM dual;


DROP SEQUENCE S3;
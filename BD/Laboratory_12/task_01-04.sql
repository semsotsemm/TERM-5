CREATE TABLE PRODUCTS (
    ID          NUMBER PRIMARY KEY,
    ITEM_NAME   VARCHAR2(50) NOT NULL,
    PRICE       NUMBER(10, 2)
);


INSERT INTO PRODUCTS (ID, ITEM_NAME, PRICE) 
                 VALUES (1, 'Ноутбук', 1200.00);
INSERT INTO PRODUCTS (ID, ITEM_NAME, PRICE) 
                 VALUES (2, 'Мышь', 25.50);
INSERT INTO PRODUCTS (ID, ITEM_NAME, PRICE)  
                 VALUES (3, 'Клавиатура', 75.00);
INSERT INTO PRODUCTS (ID, ITEM_NAME, PRICE) 
                 VALUES (4, 'Монитор', 350.00);
INSERT INTO PRODUCTS (ID, ITEM_NAME, PRICE)
                 VALUES (5, 'Наушники', 90.00);
INSERT INTO PRODUCTS (ID, ITEM_NAME, PRICE) 
                 VALUES (6, 'Веб-камера', 60.00);
INSERT INTO PRODUCTS (ID, ITEM_NAME, PRICE) 
                 VALUES (7, 'Микрофон', 110.00);
INSERT INTO PRODUCTS (ID, ITEM_NAME, PRICE)  
                 VALUES (8, 'Коврик для мыши', 15.00);
INSERT INTO PRODUCTS (ID, ITEM_NAME, PRICE) 
                 VALUES (9, 'USB-хаб', 30.00);
INSERT INTO PRODUCTS (ID, ITEM_NAME, PRICE)
                 VALUES (10, 'Колонки', 85.00);
COMMIT;


CREATE OR REPLACE TRIGGER BEFORE_OPERATOR_TRIGGER
    BEFORE INSERT OR DELETE OR UPDATE 
    ON PRODUCTS
        BEGIN
            IF INSERTING THEN
                DBMS_OUTPUT.PUT_LINE('[Trigger-12-1] Вставка.');
            ELSIF UPDATING THEN
                DBMS_OUTPUT.PUT_LINE('[Trigger-12-1] Обновление.');
            ELSIF DELETING THEN
                DBMS_OUTPUT.PUT_LINE('[Trigger-12-1] Удаление.');
            END IF;
        END;
        /


INSERT INTO PRODUCTS (ID, ITEM_NAME, PRICE)
                 VALUES (11, 'Флешка', 20.00);
UPDATE PRODUCTS SET PRICE = PRICE + 10 WHERE ID = 3;
DELETE FROM PRODUCTS WHERE ID = 11;
ROLLBACK;
CREATE OR REPLACE TRIGGER BEFORE_ROW_TRIGGER
    BEFORE INSERT OR DELETE OR UPDATE
    ON PRODUCTS
    FOR EACH ROW
        BEGIN
            IF INSERTING THEN
                DBMS_OUTPUT.PUT_LINE('[Trigger-12-2] Вставка.');
            ELSIF UPDATING THEN
                DBMS_OUTPUT.PUT_LINE('[Trigger-12-2] Обновление.');
            ELSIF DELETING THEN
                DBMS_OUTPUT.PUT_LINE('[Trigger-12-] Удаление.');
            END IF;
        END;
        /

        
INSERT INTO PRODUCTS (ID, ITEM_NAME, PRICE) 
              VALUES (11, 'Флешка', 20.00);
UPDATE PRODUCTS SET PRICE = PRICE + 10 WHERE ID IN (1, 2, 3);
DELETE FROM PRODUCTS WHERE ID = 11;
ROLLBACK;
CREATE TABLE "AUDIT" (
    OperationDate DATE DEFAULT SYSDATE,
    OperationType VARCHAR2(20),
    TriggerName   VARCHAR2(50),
    Data          VARCHAR2(400)
);


CREATE OR REPLACE TRIGGER BEFORE_OPERATOR_TRIGGER
    BEFORE INSERT OR DELETE OR UPDATE 
    ON PRODUCTS
        DECLARE
            v_op VARCHAR2(20);
        BEGIN
            IF INSERTING THEN 
                v_op := 'INSERT';
            ELSIF UPDATING THEN 
                v_op := 'UPDATE';
            ELSIF DELETING THEN
                v_op := 'DELETE';
            END IF;
        
            DBMS_OUTPUT.PUT_LINE('BEFORE_OPERATOR_TRIGGER: ' || v_op);
            INSERT INTO "AUDIT" (OperationDate, OperationType, TriggerName, Data)
                         VALUES (SYSDATE, v_op, 'BEFORE_OPERATOR_TRIGGER', 'Statement level');
        END;
        /


CREATE OR REPLACE TRIGGER BEFORE_ROW_TRIGGER
    BEFORE INSERT OR DELETE OR UPDATE 
    ON PRODUCTS
    FOR EACH ROW
        DECLARE
            v_op   VARCHAR2(20);
            v_data VARCHAR2(400);
        BEGIN
            IF INSERTING THEN
                v_op   := 'INSERT';
                v_data := 'NEW: ' || :NEW.ID || ', ' || :NEW.ITEM_NAME || ', ' || :NEW.PRICE;
            ELSIF UPDATING THEN
                v_op   := 'UPDATE';
                v_data := 'OLD: ' || :OLD.ID || ', ' || :OLD.PRICE || ' -> NEW: ' || :NEW.ID || ', ' || :NEW.PRICE;
            ELSIF DELETING THEN
                v_op   := 'DELETE';
                v_data := 'OLD: ' || :OLD.ID || ', ' || :OLD.ITEM_NAME || ', ' || :OLD.PRICE;
            END IF;
        
            DBMS_OUTPUT.PUT_LINE('BEFORE_ROW_TRIGGER: ' || v_op);
            INSERT INTO "AUDIT" (OperationDate, OperationType, TriggerName, Data)
                         VALUES (SYSDATE, v_op, 'BEFORE_ROW_TRIGGER', v_data);
        END;
        /


CREATE OR REPLACE TRIGGER AFTER_ROW_TRIGGER
    AFTER INSERT OR DELETE OR UPDATE 
    ON PRODUCTS
    FOR EACH ROW
        DECLARE
            v_op   VARCHAR2(20);
            v_data VARCHAR2(400);
        BEGIN
            IF INSERTING THEN
                v_op   := 'INSERT';
                v_data := 'NEW: ' || :NEW.ID || ', ' || :NEW.ITEM_NAME || ', ' || :NEW.PRICE;
            ELSIF UPDATING THEN
                v_op   := 'UPDATE';
                v_data := 'OLD: ' || :OLD.ID || ', ' || :OLD.PRICE || ' -> NEW: ' || :NEW.ID || ', ' || :NEW.PRICE;
            ELSIF DELETING THEN
                v_op   := 'DELETE';
                v_data := 'OLD: ' || :OLD.ID || ', ' || :OLD.ITEM_NAME || ', ' || :OLD.PRICE;
            END IF;
        
            DBMS_OUTPUT.PUT_LINE('AFTER_ROW_TRIGGER: ' || v_op);
            INSERT INTO "AUDIT" (OperationDate, OperationType, TriggerName, Data)
                         VALUES (SYSDATE, v_op, 'AFTER_ROW_TRIGGER', v_data);
        END;
        /


CREATE OR REPLACE TRIGGER AFTER_OPERATOR_TRIGGER
    AFTER INSERT OR DELETE OR UPDATE
    ON PRODUCTS
        DECLARE
            v_op VARCHAR2(20);
        BEGIN
            IF INSERTING THEN
                v_op := 'INSERT';
            ELSIF UPDATING THEN 
                v_op := 'UPDATE';
            ELSIF DELETING THEN 
                v_op := 'DELETE';
            END IF;
        
            DBMS_OUTPUT.PUT_LINE('AFTER_OPERATOR_TRIGGER: ' || v_op);
            INSERT INTO "AUDIT" (OperationDate, OperationType, TriggerName, Data)
                         VALUES (SYSDATE, v_op, 'AFTER_OPERATOR_TRIGGER', 'Statement level');
        END;
        /


INSERT INTO PRODUCTS (ID, ITEM_NAME, PRICE) 
                 VALUES (11, 'Флешка', 20.00);
UPDATE PRODUCTS SET PRICE = PRICE + 10 WHERE ID IN (1, 2);
DELETE FROM PRODUCTS WHERE ID = 11;
SELECT * 
    FROM "AUDIT";
ROLLBACK;

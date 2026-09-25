CREATE TABLE my_accounts(id number(3), card_number varchar2(20), balance number(8), cvv number(3));
COMMIT;


INSERT INTO my_accounts (id, card_number, balance, cvv) VALUES (1, '4829 1940 8274 9102', 45230, 814);
INSERT INTO my_accounts (id, card_number, balance, cvv) VALUES (2, '5193 8472 0192 4817', 128450, 295);
INSERT INTO my_accounts (id, card_number, balance, cvv) VALUES (3, '4029 6718 9934 1056', 3200, 603);
COMMIT;


SELECT * 
    FROM my_accounts;


CREATE VIEW Vaccount as 
    SELECT card_number, cvv
    FROM my_accounts;    
COMMIT;


SELECT * 
    FROM Vaccount;


DROP VIEW Vaccount;
DROP TABLE my_accounts PURGE;
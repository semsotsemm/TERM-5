SET SERVEROUTPUT ON;

DECLARE
    TYPE t_address IS RECORD (
        city    VARCHAR2(50),
        street  VARCHAR2(100),
        zip     NUMBER(6)
    );

    TYPE t_teacher_info IS RECORD (
        full_name    VARCHAR2(100),
        department   VARCHAR2(20),
        home_address t_address 
    );

    v_addr1   t_address;
    v_addr2   t_address;
    v_teacher t_teacher_info;
BEGIN
    v_addr1.city := 'Минск';
    v_addr1.street := 'ул. Свердлова, 13а';
    v_addr1.zip := 220050;

    v_addr2 := v_addr1;
    v_addr2.street := 'ул. Бобруйская, 9'; 

    v_teacher.full_name := 'Смелов В.В.';
    v_teacher.department := 'ИСиТ';
    
    v_teacher.home_address := v_addr1;
    
    v_teacher.home_address.zip := 220000;

    DBMS_OUTPUT.PUT_LINE('Преподаватель: ' || v_teacher.full_name);
    DBMS_OUTPUT.PUT_LINE('Город (из вложенной записи): ' || v_teacher.home_address.city);
    DBMS_OUTPUT.PUT_LINE('Улица оригинала (v_addr1): ' || v_addr1.street);
    DBMS_OUTPUT.PUT_LINE('Улица копии (v_addr2): ' || v_addr2.street);
END;
/
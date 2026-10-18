DECLARE
    v_int1 NUMBER(10) := 15;
    v_int2 NUMBER(10) := 4;
    
    v_fix NUMBER(5,2) := 123.45;
    v_fix_neg NUMBER(5,-2) := 12345;

    v_bfloat BINARY_FLOAT := 1.5f;
    v_bdouble BINARY_DOUBLE := 2.5d;
    
    v_sci NUMBER := 1.2E3;
    v_bool BOOLEAN := TRUE;    
BEGIN
    DBMS_OUTPUT.PUT_LINE('Сложение: ' || (v_int1 + v_int2));
    DBMS_OUTPUT.PUT_LINE('Остаток от деления: ' || MOD(v_int1, v_int2));
    DBMS_OUTPUT.PUT_LINE('Фиксированная точка: ' || v_fix);
    DBMS_OUTPUT.PUT_LINE('Отрицательный масштаб: ' || v_fix_neg);
    DBMS_OUTPUT.PUT_LINE('Бинарный флоат: ' || v_bfloat);
    DBMS_OUTPUT.PUT_LINE('Бинарный дабл: ' || v_bdouble);
    DBMS_OUTPUT.PUT_LINE('Возведение в 10 степень: ' || v_sci);
    IF v_bool THEN
        DBMS_OUTPUT.PUT_LINE('Бул: TRUE');
    END IF; 
END;
/
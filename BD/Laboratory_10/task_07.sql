SET SERVEROUTPUT ON;

DECLARE
    CURSOR c_avg_salary IS
        SELECT 
            p.FACULTY,
            t.PULPIT,
            FLOOR(AVG(t.SALARY)) AS AVG_SALARY,
            GROUPING(p.FACULTY) AS is_total_all,
            GROUPING(t.PULPIT) AS is_total_faculty
        FROM 
            TEACHER t
        JOIN 
            PULPIT p ON t.PULPIT = p.PULPIT
        GROUP BY 
            ROLLUP(p.FACULTY, t.PULPIT)
        ORDER BY 
            is_total_all, p.FACULTY, is_total_faculty, t.PULPIT;
BEGIN
    DBMS_OUTPUT.PUT_LINE('--- Средняя заработная плата ---');
    
    FOR v_rec IN c_avg_salary LOOP
        IF v_rec.is_total_all = 1 THEN
            DBMS_OUTPUT.PUT_LINE('=========================================');
            DBMS_OUTPUT.PUT_LINE('ИТОГО В ЦЕЛОМ: ' || v_rec.AVG_SALARY || ' руб.');
            
        ELSIF v_rec.is_total_faculty = 1 THEN
            DBMS_OUTPUT.PUT_LINE('-----------------------------------------');
            DBMS_OUTPUT.PUT_LINE('СРЕДНЕЕ ПО ФАКУЛЬТЕТУ ' || v_rec.FACULTY || ': ' || v_rec.AVG_SALARY || ' руб.');
            
        ELSE
            DBMS_OUTPUT.PUT_LINE('Кафедра ' || v_rec.PULPIT || ': ' || v_rec.AVG_SALARY || ' руб.');
        END IF;
    END LOOP;
END;
/
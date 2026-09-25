-- Определить количество сегментов --
SELECT extent_id, bytes, blocks 
    FROM user_extents 
    WHERE segment_name = 'AAR_T1'
    ORDER BY extent_id;

SELECT 
    COUNT(extent_id) AS total_extents, 
    SUM(bytes) AS total_bytes, 
    SUM(blocks) AS total_blocks 
    FROM user_extents 
    WHERE segment_name = 'AAR_T1';
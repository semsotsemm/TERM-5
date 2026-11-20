using UnityEngine;

public class tank_script : MonoBehaviour
{
    [Header("Настройки мыши и скорости")]
    public float mouse_sensivity = 2f;
    public float move_speed = 10f;
    public float turn_speed = 60f;
    public float tower_smooth_speed = 5f;
    public float barrel_smooth_speed = 5f;

    [Header("Ссылки на объекты")]
    public Transform tank_tower;
    public Transform tank_barrel;
    public Transform camera_pivot;

    [Header("Ограничения углов камеры (вверх / вниз)")]
    public float min_pitch = -10f;
    public float max_pitch = 20f;

    [Header("Ограничения и настройка Дула")]
    [Tooltip("Во сколько раз угол дула меньше угла камеры (0.5 = в 2 раза меньше)")]
    public float barrel_pitch_multiplier = 0.5f;
    public float barrel_min_pitch = -2f;
    public float barrel_max_pitch = 2f;
    [Tooltip("0 = X, 1 = Y, 2 = Z. Переключи в Play Mode, чтобы дуло качалось строго вверх-вниз")]
    public int barrel_axis = 0;
    public bool invert_barrel = false;
    [Tooltip("Подкрути эти значения в Play Mode, если дуло изначально стоит криво относительно башни")]
    public Vector3 barrel_offset = Vector3.zero;

    private float current_cam_pitch = 0f;
    private float target_cam_pitch = 0f;
    private float current_barrel_pitch = 0f;
    private Quaternion barrel_initial_local_rot;

    void Start()
    {
        Cursor.lockState = CursorLockMode.Locked;
        Cursor.visible = false;

        if (tank_barrel != null && tank_tower != null)
        {
            if (tank_barrel.parent != tank_tower)
            {
                tank_barrel.SetParent(tank_tower, true);
            }
            barrel_initial_local_rot = tank_barrel.localRotation;
        }
        else if (tank_barrel != null)
        {
            barrel_initial_local_rot = tank_barrel.localRotation;
        }

        if (move_speed < 1f) move_speed = 10f;
        if (turn_speed < 5f) turn_speed = 60f;
    }

    void Update()
    {
        float mouse_x = Input.GetAxis("Mouse X") * mouse_sensivity;
        float mouse_y = Input.GetAxis("Mouse Y") * mouse_sensivity;

        // --- 1. ВРАЩЕНИЕ КАМЕРЫ ---
        if (camera_pivot != null)
        {
            camera_pivot.Rotate(camera_pivot.right, -current_cam_pitch, Space.World);
            camera_pivot.Rotate(Vector3.up, mouse_x, Space.World);

            target_cam_pitch = Mathf.Clamp(target_cam_pitch - mouse_y, min_pitch, max_pitch);
            current_cam_pitch = target_cam_pitch;

            camera_pivot.Rotate(camera_pivot.right, current_cam_pitch, Space.World);
        }

        // --- 2. ПОВОРОТ БАШНИ И СТВОЛА (когда ПКМ отпущен) ---
        if (!Input.GetMouseButton(1))
        {
            if (tank_tower != null && camera_pivot != null)
            {
                float angle_difference = Mathf.DeltaAngle(tank_tower.eulerAngles.y, camera_pivot.eulerAngles.y);
                float rotation_step = Mathf.MoveTowardsAngle(0f, angle_difference, tower_smooth_speed * 10f * Time.deltaTime);
                tank_tower.Rotate(Vector3.up, rotation_step, Space.World);
            }

            if (tank_barrel != null)
            {
                // Умножаем угол камеры на 0.5f (в 2 раза меньше) и жестко ограничиваем лимитами дула
                float clamped_barrel_target = Mathf.Clamp(target_cam_pitch * barrel_pitch_multiplier, barrel_min_pitch, barrel_max_pitch);
                float desired_pitch = invert_barrel ? -clamped_barrel_target : clamped_barrel_target;

                current_barrel_pitch = Mathf.MoveTowards(current_barrel_pitch, desired_pitch, barrel_smooth_speed * 10f * Time.deltaTime);
            }
        }

        // Применяем поворот дула каждый кадр с учетом выравнивания (barrel_offset)
        if (tank_barrel != null)
        {
            Vector3 rot_axis = Vector3.right;
            if (barrel_axis == 1) rot_axis = Vector3.up;
            if (barrel_axis == 2) rot_axis = Vector3.forward;

            Quaternion offset_rot = Quaternion.Euler(barrel_offset);
            Quaternion pitch_rot = Quaternion.AngleAxis(current_barrel_pitch, rot_axis);

            tank_barrel.localRotation = barrel_initial_local_rot * offset_rot * pitch_rot;
        }

        if (Input.GetKeyDown(KeyCode.Escape))
        {
            Cursor.lockState = CursorLockMode.None;
            Cursor.visible = true;
        }

        // --- 3. ДВИЖЕНИЕ И ПОВОРОТ КОРПУСА ---
        float move_input = Input.GetAxis("Vertical");
        float turn_input = Input.GetAxis("Horizontal");

        float move_step = move_input * move_speed * Time.deltaTime;
        transform.Translate(0f, move_step, 0f, Space.Self);

        float turn_step = turn_input * turn_speed * Time.deltaTime;
        transform.Rotate(0f, 0f, turn_step, Space.Self);
    }
}
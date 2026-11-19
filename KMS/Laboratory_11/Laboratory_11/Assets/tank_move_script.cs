using UnityEngine;

public class tank_move_script : MonoBehaviour
{
    public float mouse_sensivity = 2f;
    private float move_speed = 10f;
    public float turn_speed = 60f;
    public float tower_smooth_speed = 2.5f;
    public float barrel_smooth_speed = 1.2f;


    public Transform tank_tower;
    public Transform tank_barrel;
    public Transform camera_pivot;
    public AudioSource engine_audio;


    public float min_camera_angel = -35f;
    public float max_camera_angel = 65f;
    public float start_camera_angel = 18f;
    public float camera_height = 4f;


    public float barrel_min_angel = -6f;
    public float barrel_max_angel = 20f;


    private float move_input;
    private float turn_input;
    private float camera_horizontal_angle;
    private float camera_vertical_angle = 18f;
    private float tower_horizontal_angle;
    private float barrel_vertical_angel;


    void Start()
    {
        camera_horizontal_angle = Mathf.Atan2(transform.forward.x, transform.forward.z) * Mathf.Rad2Deg; // 57.2958
        tower_horizontal_angle = camera_horizontal_angle;

        Cursor.lockState = CursorLockMode.Locked;
        Cursor.visible = false;
    }

    void Update()
    {
        if (Input.GetKeyDown(KeyCode.Escape))
        {
            Cursor.lockState = CursorLockMode.None;
            Cursor.visible = true;
        }
        if (Cursor.lockState == CursorLockMode.Locked)
        {
            camera_horizontal_angle = Mathf.Repeat(camera_horizontal_angle + Input.GetAxis("Mouse X") * mouse_sensivity, 360f);
            camera_vertical_angle = Mathf.Clamp(camera_vertical_angle - Input.GetAxis("Mouse Y") * mouse_sensivity, min_camera_angel, max_camera_angel);
        }

        move_input = Input.GetAxis("Vertical");
        turn_input = Input.GetAxis("Horizontal");

        transform.Translate(Vector3.forward * move_input * move_speed * Time.deltaTime, Space.Self);
        transform.Rotate(0f, turn_input * turn_speed * Time.deltaTime, 0f, Space.World);

        if (camera_pivot != null)
        {
            camera_pivot.position = transform.position + Vector3.up * camera_height;
            camera_pivot.rotation = Quaternion.Euler(camera_vertical_angle, camera_horizontal_angle, 0f);
        }

        if (!Input.GetMouseButton(1))
        {
            tower_horizontal_angle = Mathf.MoveTowardsAngle(tower_horizontal_angle, camera_horizontal_angle, tower_smooth_speed * 10f * Time.deltaTime);
            float target = -camera_vertical_angle;
            target = Mathf.Clamp(target, barrel_min_angel, barrel_max_angel);
            barrel_vertical_angel = Mathf.MoveTowards(barrel_vertical_angel, target, barrel_smooth_speed * 10f * Time.deltaTime);
        }

        if (tank_tower != null)
        {
            tank_tower.rotation = Quaternion.Euler(0f, tower_horizontal_angle + 180f, 0f);
        }

        if (tank_barrel != null)
        {
            tank_barrel.localRotation = Quaternion.Euler(0f, 180f, 0f) * Quaternion.AngleAxis(-barrel_vertical_angel, Vector3.right);
        }

        UpdateEngineSound(Time.deltaTime);
    }

    void UpdateEngineSound(float delta_time)
    {
        float activity = Mathf.Max(Mathf.Abs(move_input), Mathf.Abs(turn_input));
        if (activity > 0.01f && !engine_audio.isPlaying)
        {
            engine_audio.Play();
        }

        engine_audio.volume = Mathf.MoveTowards(engine_audio.volume, activity > 0.03f ? 0.2f : 0f, delta_time * 0.3f);

        if (activity <= 0.03f && engine_audio.volume <= 0f && engine_audio.isPlaying)
        {
            engine_audio.Stop();
        }
    }
}
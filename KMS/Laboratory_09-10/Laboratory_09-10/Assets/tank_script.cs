using UnityEngine;

public class tank_script : MonoBehaviour
{
    public float mouse_sensivity = 2f;
    public float move_speed = 10f;
    public float turn_speed = 60f;
    public float tower_smooth_speed = 2.5f;
    public float barrel_smooth_speed = 1.2f;

    public Transform tank_tower;
    public Transform tank_barrel;
    public Transform camera_pivot;
    public AudioSource engine_audio;

    public float min_pitch = -35f;
    public float max_pitch = 65f;
    public float start_pitch = 18f;
    public float camera_height = 4f;

    public float barrel_pitch_multiplier = 1f;
    public float barrel_min_pitch = -6f;
    public float barrel_max_pitch = 20f;
    public int barrel_axis = 0;
    public bool invert_barrel = false;
    public Vector3 barrel_offset = Vector3.zero;

    private Rigidbody tank_body;
    private float move_input;
    private float turn_input;
    private float camera_yaw;
    private float target_cam_pitch;
    private float tower_yaw;
    private float current_barrel_pitch;
    private Quaternion tower_rotation_offset;
    private Quaternion barrel_initial_local_rot;

    void Start()
    {
        tank_body = GetComponent<Rigidbody>();
        camera_yaw = Mathf.Atan2(transform.forward.x, transform.forward.z) * Mathf.Rad2Deg;
        tower_yaw = camera_yaw;
        target_cam_pitch = Mathf.Clamp(start_pitch, min_pitch, max_pitch);
        if (tank_tower != null)
        { 
            tower_rotation_offset = Quaternion.Inverse(Quaternion.Euler(0f, tower_yaw, 0f)) * tank_tower.rotation;
        }
        if (tank_barrel != null)
        { 
            barrel_initial_local_rot = tank_barrel.localRotation;
        }
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
            camera_yaw = Mathf.Repeat(camera_yaw + Input.GetAxis("Mouse X") * mouse_sensivity, 360f);
            target_cam_pitch = Mathf.Clamp(target_cam_pitch - Input.GetAxis("Mouse Y") * mouse_sensivity, min_pitch, max_pitch);
        }
        move_input = Input.GetAxis("Vertical");
        turn_input = Input.GetAxis("Horizontal");

        transform.Translate(Vector3.forward * move_input * move_speed * Time.deltaTime, Space.Self);
        transform.Rotate(0f, turn_input * turn_speed * Time.deltaTime, 0f, Space.World);

        if (camera_pivot != null)
        {
            camera_pivot.position = transform.position + Vector3.up * camera_height;
            camera_pivot.rotation = Quaternion.Euler(target_cam_pitch, camera_yaw, 0f);
        }

        if (!Input.GetMouseButton(1)) 
        {
            tower_yaw = Mathf.MoveTowardsAngle(tower_yaw, camera_yaw, tower_smooth_speed * 10f * Time.deltaTime);
            float target = -target_cam_pitch * barrel_pitch_multiplier;
            if (invert_barrel) target = -target;
            target = Mathf.Clamp(target, barrel_min_pitch, barrel_max_pitch);
            current_barrel_pitch = Mathf.MoveTowards(current_barrel_pitch, target, barrel_smooth_speed * 10f * Time.deltaTime);
        }

        if (tank_tower != null)
        {
            tank_tower.rotation = Quaternion.Euler(0f, tower_yaw, 0f) * tower_rotation_offset;
        }

        if (tank_barrel != null)
        {
            Vector3 axis = barrel_axis == 1 ? Vector3.up : barrel_axis == 2 ? Vector3.forward : Vector3.right;
            tank_barrel.localRotation = barrel_initial_local_rot * Quaternion.Euler(barrel_offset) * Quaternion.AngleAxis(-current_barrel_pitch, axis);
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

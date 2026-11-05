using UnityEngine;

public class wall_trigger : MonoBehaviour
{
    public Rigidbody wall;
    public float rotation_speed = 35f;
    private float last_rotation_time = -1f;

    void OnTriggerStay(Collider other)
    {
        if (wall == null || other.GetComponentInParent<tank_script>() == null) return;
        // Несколько коллайдеров одного танка не должны ускорять вращение стенки.
        if (Mathf.Approximately(last_rotation_time, Time.fixedTime)) return;
        last_rotation_time = Time.fixedTime;
        wall.MoveRotation(Quaternion.AngleAxis(rotation_speed * Time.fixedDeltaTime, Vector3.up) * wall.rotation);
    }
}

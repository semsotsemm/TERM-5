using UnityEngine;

public class wall_trigger : MonoBehaviour
{
    public Rigidbody wall;
    public float rotation_speed = 35f;

    void OnTriggerStay(Collider other)
    {
        wall.MoveRotation(Quaternion.AngleAxis(rotation_speed * Time.fixedDeltaTime, Vector3.up) * wall.rotation);
    }
}

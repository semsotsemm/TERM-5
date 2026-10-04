using UnityEngine;

public class Rotation_Euler : MonoBehaviour
{
    public float speed = 60f;

    private void Update()
    {
        Vector3 angles = transform.eulerAngles;

        angles.x += speed * Time.deltaTime;
        angles.z += speed * Time.deltaTime;

        transform.eulerAngles = angles;
    }
}
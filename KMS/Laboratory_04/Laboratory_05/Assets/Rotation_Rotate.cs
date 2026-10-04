using UnityEngine;

public class Rotation_Rotate : MonoBehaviour
{
    public float speed = 60f;

    private void Update()
    {
        transform.Rotate(0, speed * Time.deltaTime, 0);
    }
}
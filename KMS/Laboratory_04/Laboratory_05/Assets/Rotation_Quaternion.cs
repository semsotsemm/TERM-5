using UnityEngine;

public class Rotation_Quaternion : MonoBehaviour
{
    public float speed = 60f;

    private Quaternion startRotation;
    private float angle;

    private void Start()
    {
        startRotation = transform.rotation;
    }

    private void Update()
    {
        angle += speed * Time.deltaTime;

        Quaternion rotationX = Quaternion.AngleAxis(angle, Vector3.right);

        Quaternion rotationZ = Quaternion.AngleAxis(angle, Vector3.forward);

        transform.rotation = startRotation * rotationX * rotationZ;
    }
}
using UnityEngine;

public class PlayerMove : MonoBehaviour
{
    void Update()
    {
        float h = Input.GetAxis("Horizontal") * 6.0f * Time.deltaTime;
        float v = Input.GetAxis("Vertical") * 6.0f * Time.deltaTime;
        float mouseX = Input.GetAxis("Mouse X") * 3.0f;

        transform.Translate(h, 0, v);
        transform.Rotate(0, mouseX, 0);
    }
}
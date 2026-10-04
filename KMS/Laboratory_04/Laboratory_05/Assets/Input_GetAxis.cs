using UnityEngine;
public class Input_GetAxis : MonoBehaviour
{
    public float speed = 0.05f;
    public float mouseSensitivity = 2f;
    float angleY = 0f;

    void Update()
    {
        float x = Input.GetAxis("Horizontal"); 
        float z = Input.GetAxis("Vertical");   

        transform.Translate(x * speed, 0, z * speed);


        float mouseX = Input.GetAxis("Mouse X");
        float mouseY = Input.GetAxis("Mouse Y");

        transform.Rotate(0, mouseX * mouseSensitivity, 0);

        angleY -= mouseY * mouseSensitivity;
        angleY = Mathf.Clamp(angleY, 0f, 90f); 
        
        transform.localEulerAngles = new Vector3(angleY, transform.localEulerAngles.y, 0);
    }
}
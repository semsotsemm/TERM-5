using UnityEngine;

public class RobotMove : MonoBehaviour
{
    void Update()
    {
        float speed = 5.0f * Time.deltaTime;
        if (Input.GetKey(KeyCode.E)) transform.Translate(0, 0, speed);  
        if (Input.GetKey(KeyCode.R)) transform.Translate(0, 0, -speed); 
        if (Input.GetKey(KeyCode.T)) transform.Translate(-speed, 0, 0); 
        if (Input.GetKey(KeyCode.Y)) transform.Translate(speed, 0, 0);  
    }
}
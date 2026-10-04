using UnityEngine;

public class ScriptPosition : MonoBehaviour
{
    public float speed_x = 0.08f;
    public float speed_y = 0.02f;
    public float speed_z = -0.01f;

    void Update() 
    {
        transform.position += new Vector3(speed_x, speed_y, speed_z);
    }
}

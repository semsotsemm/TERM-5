using UnityEngine;

public class WallLightTrigger : MonoBehaviour
{
    public Light Point;

    void OnTriggerEnter(Collider col)
    {
        if (col.name == "player")
        {
            Point.enabled = true; 
        }
    }

    void OnTriggerExit(Collider col)
    {
        if (col.name == "player")
        {
            Point.enabled = false; 
        }
    }
}
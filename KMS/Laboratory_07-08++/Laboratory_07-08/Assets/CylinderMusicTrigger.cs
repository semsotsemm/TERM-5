using UnityEngine;

public class CylinderMusicTrigger : MonoBehaviour
{
    public GameObject cylinder; 
    public Light Point1;        
    public Light Point2;        
    public Light Point3;        

    void OnTriggerStay(Collider col)
    {
        if (col.name == "player" || col.name == "robot")
        {
            cylinder.transform.Rotate(0, 90.0f * Time.deltaTime, 0, Space.Self);

            float newIntensity = Mathf.PingPong(Time.time * 10.0f, 14.0f) + 1.0f;
            Point1.intensity = newIntensity;
            Point2.intensity = newIntensity;
            Point3.intensity = newIntensity;
        }
    }
}
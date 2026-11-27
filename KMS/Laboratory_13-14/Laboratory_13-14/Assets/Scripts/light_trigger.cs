using UnityEngine;

public class light_trigger : MonoBehaviour
{
    public Light point_light;
    private float active_intensity = 10000f;

    void Start()
    {
        if (point_light != null)
        {
            point_light.intensity = 0f; 
        }
    }

    void OnTriggerEnter(Collider other)
    {
        point_light.intensity = active_intensity;
    }

    void OnTriggerExit(Collider other)
    {
        point_light.intensity = 0f;
    }
}
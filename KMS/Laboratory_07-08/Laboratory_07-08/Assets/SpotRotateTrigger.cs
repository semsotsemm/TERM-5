using UnityEngine;

public class SpotRotateTrigger : MonoBehaviour
{
    public GameObject Spot; 

    void OnTriggerStay(Collider col)
    {
        if (col.name == "player")
        {
            Spot.transform.Rotate(0, 100.0f * Time.deltaTime, 0, Space.World);
        }
    }
}
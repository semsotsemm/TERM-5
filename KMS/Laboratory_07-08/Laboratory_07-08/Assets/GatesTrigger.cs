using UnityEngine;

public class GatesTrigger : MonoBehaviour
{
    public GameObject leftDoor;   
    public GameObject rightDoor;  
    public GameObject flyingCube; 

    void OnTriggerEnter(Collider col)
    {
        if (col.name == "player" || col.name == "robot")
        {
            leftDoor.transform.Translate(-0.8f, 0, 0);
            rightDoor.transform.Translate(0.8f, 0, 0);
        }
    }

    void OnTriggerExit(Collider col)
    {
        if (col.name == "player" || col.name == "robot")
        {
            leftDoor.transform.Translate(0.8f, 0, 0);
            rightDoor.transform.Translate(-0.8f, 0, 0);
        }
    }

    void OnTriggerStay(Collider col)
    {
        if (col.name == "robot")
        {
            flyingCube.transform.Rotate(0, 150.0f * Time.deltaTime, 0, Space.World);
            flyingCube.transform.Translate(0, 0, -1.0f * Time.deltaTime, Space.World);
        }
    }
}
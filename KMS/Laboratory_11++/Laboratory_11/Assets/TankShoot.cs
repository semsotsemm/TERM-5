using UnityEngine;

public class TankShoot : MonoBehaviour
{
    public GameObject corePrefab;

    public float barrelLength = 9.0f;

    void Update()
    {
        if (Input.GetKeyDown(KeyCode.Space))
        {
            Vector3 spawnPosition = transform.position + transform.forward * barrelLength;

            Quaternion spawnRotation = transform.rotation * Quaternion.Euler(90f, 0f, 0f);
            Instantiate(corePrefab, spawnPosition, spawnRotation);
        }
    }
}
using System.Collections;
using UnityEngine;

public class bomb_dropper : MonoBehaviour
{
    public GameObject bomb_prefab;
    public int bomb_count = 16;
    public float drop_height = 35f;
    public float area_size = 15f; 
    public float forward_distance = 20f; 

    void Update()
    {
        if (Input.GetKeyDown(KeyCode.Q))
        {
            StartCoroutine(Bombing());
        }
    }

    IEnumerator Bombing()
    {
        Vector3 center = transform.position + transform.forward * forward_distance;
        center.y = transform.position.y + drop_height;

        for (int i = 0; i < bomb_count; i++)
        {
            float random_offset_x = Random.Range(-area_size, area_size);
            float random_offset_z = Random.Range(-area_size, area_size);

            Vector3 spawn_pos = center + transform.right * random_offset_x + transform.forward * random_offset_z;

            Instantiate(bomb_prefab, spawn_pos, Quaternion.identity);

            yield return new WaitForSeconds(0.1f);
        }

    }
}
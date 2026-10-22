using JetBrains.Annotations;
using UnityEngine;

public class PlaneScript : MonoBehaviour
{
    MeshRenderer mesh_renderer;
    public GameObject prefub1;

    void Start()
    {
        mesh_renderer = GetComponent<MeshRenderer>();
    }

    void Update()
    {
        float min_x = mesh_renderer.bounds.min.x;
        float max_x = mesh_renderer.bounds.max.x;
        float min_z = mesh_renderer.bounds.min.z;
        float max_z = mesh_renderer.bounds.max.z;


        float rand_x = Random.Range(min_x, max_x);  
        float rand_z = Random.Range(min_z, max_z);
        float pos_y = transform.position.y + 5.0f;
        Vector3 spawn_pos = new Vector3(rand_x, pos_y, rand_z);

        if (Input.GetKeyDown(KeyCode.Q)) 
        {
            GameObject cube = GameObject.CreatePrimitive(PrimitiveType.Cube);
            cube.transform.position = spawn_pos;
            cube.AddComponent<Rigidbody>();
        }

        if (Input.GetKeyDown(KeyCode.Space)) 
        {
            Instantiate(prefub1, spawn_pos, Quaternion.identity);
        }

        if (Input.GetKeyDown(KeyCode.W))
        {
            transform.Rotate(0f, 0f, 0.5f);
        }
    }
} 

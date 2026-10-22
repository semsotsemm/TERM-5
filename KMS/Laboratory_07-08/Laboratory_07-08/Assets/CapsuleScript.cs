using UnityEngine;

public class CapsuleScript : MonoBehaviour
{
    public Texture texture_on_hit;
    public Texture texture_by_key;
    public GameObject cube_1;

    void Update()
    {
        float move_x = Input.GetAxis("Horizontal") * 5.0f * Time.deltaTime;
        float move_z = Input.GetAxis("Vertical") * 5.0f * Time.deltaTime;

        transform.Translate(move_x, 0, move_z);

        float mouse_x = Input.GetAxis("Mouse X") * 1.0f;
        transform.Rotate(0, mouse_x, 0);

        if (Input.GetKeyDown(KeyCode.T) || Input.GetKeyDown(KeyCode.Space))
        {
            if (cube_1 != null && texture_by_key != null) 
            {
                cube_1.gameObject.GetComponent<Renderer>().material.SetTexture("_BaseMap", texture_by_key);
                //cube_1.gameObject.GetComponent<Renderer>().material.mainTexture = texture_by_key;
            }     
        }

    }

    void OnCollisionEnter(Collision col) 
    {
        if (col.gameObject.name != "Plane") 
        {
            Color randColor = new Color(Random.value, Random.value, Random.value);

            col.gameObject.GetComponent<Renderer>().material.color = randColor;
            col.gameObject.GetComponent<Renderer>().material.SetTexture("_BaseMap", texture_on_hit);
            //col.gameObject.GetComponent<Renderer>().material.mainTexture = texture_on_hit;
        }
    }
}

using UnityEngine;

public class bomb_script : MonoBehaviour
{
    public AudioClip impact_sound;
    public float lifetime = 15f;

    void Start()
    {
        Destroy(gameObject, lifetime);
    }

    void OnCollisionEnter(Collision collision)
    {
        if (impact_sound != null)
        {
            AudioSource.PlayClipAtPoint(impact_sound, Camera.main.transform.position);
        }
        Destroy(gameObject);
    }
}
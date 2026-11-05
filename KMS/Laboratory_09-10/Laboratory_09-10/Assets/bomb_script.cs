using UnityEngine;

public class bomb_script : MonoBehaviour
{
    public AudioClip impact_sound;
    [HideInInspector] public Collider ground;
    public float lifetime = 15f;
    private bool landed;

    void Start()
    {
        Destroy(gameObject, lifetime);
    }

    void OnCollisionEnter(Collision collision)
    {
        if (landed || collision.collider != ground) return;
        landed = true;
        if (impact_sound != null)
        {
            GameObject sound_object = new GameObject("Bomb impact sound");
            sound_object.transform.position = collision.GetContact(0).point;
            AudioSource source = sound_object.AddComponent<AudioSource>();
            source.spatialBlend = 0.3f;
            source.minDistance = 10f;
            source.maxDistance = 100f;
            source.PlayOneShot(impact_sound, 0.8f);
            Destroy(sound_object, impact_sound.length + 0.1f);
        }
        Destroy(gameObject);
    }
}

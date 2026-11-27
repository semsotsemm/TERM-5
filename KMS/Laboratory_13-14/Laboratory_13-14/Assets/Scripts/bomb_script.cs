using UnityEngine;

public class bomb_script : MonoBehaviour
{
    public AudioClip impact_sound;
    public GameObject explosion;
    public float lifetime = 15f;

    private bool exploded;

    void Start()
    {
        Destroy(gameObject, lifetime);
    }

    void OnCollisionEnter(Collision collision)
    {
        if (exploded) return;
        exploded = true;

        ContactPoint contact = collision.GetContact(0);
        CoreMove.PlayImpact(impact_sound, contact.point, .29f, 95f);
        if (explosion != null)
        {
            GameObject blast = Instantiate(explosion, contact.point + contact.normal * .06f,
                Quaternion.LookRotation(contact.normal));
            blast.transform.localScale *= 1.65f;
        }
        Destroy(gameObject);
    }
}

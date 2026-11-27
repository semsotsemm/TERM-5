using UnityEngine;

public class CoreMove : MonoBehaviour
{
    public float speed = 160.0f;
    public float lifeTime = 3.0f;
    public float hitForce = 20.0f;
    public GameObject explosion1;

    public AudioClip impactSound;
    [HideInInspector] public Transform botOwner;

    private bool hasHit = false;

    void Start()
    {
        Destroy(gameObject, lifeTime);
    }

    void Update()
    {
        if (hasHit) return;

        float step = speed * Time.deltaTime;

        if (botOwner != null)
        {
            if (FindHit(transform.position, transform.up, step, botOwner, out RaycastHit botHit))
            {
                HitGoal(botHit.collider.gameObject, botHit.point, botHit.normal);
                return;
            }
        }
        else if (Physics.Raycast(transform.position, transform.up, out RaycastHit hit, step))
        {
            if (hit.collider.CompareTag("goal"))
            {
                HitGoal(hit.collider.gameObject, hit.point, hit.normal);
                return;
            }
        }

        transform.Translate(Vector3.up * step);
    }

    void OnCollisionEnter(Collision collision)
    {
        if (botOwner != null)
        {
            if (!collision.transform.IsChildOf(botOwner))
            {
                HitContact(collision);
            }
            return;
        }

        if (collision.gameObject.CompareTag("goal"))
        {
            HitContact(collision);
        }
    }

    void HitContact(Collision collision)
    {
        ContactPoint contact = collision.GetContact(0);
        HitGoal(collision.gameObject, contact.point, contact.normal);
    }

    void HitGoal(GameObject target, Vector3 hitPoint, Vector3 normal)
    {
        if (hasHit) return;
        hasHit = true;

        GetComponent<Renderer>().enabled = false;
        GetComponent<Collider>().enabled = false;

        AudioSource targetAudio = target.CompareTag("Player") ? null : target.GetComponent<AudioSource>();
        if (targetAudio != null && targetAudio.clip != null)
        {
            PlayImpact(targetAudio.clip, hitPoint, targetAudio.volume, 85f);
        }
        else
        {
            PlayImpact(impactSound, hitPoint, .48f, 85f);
        }

        Rigidbody targetRb = target.GetComponent<Rigidbody>();
        if (targetRb != null)
        {
            targetRb.AddForceAtPosition(transform.up * hitForce, hitPoint, ForceMode.Impulse);
        }

        if (explosion1 != null)
        {
            Instantiate(explosion1, hitPoint + normal * .06f, Quaternion.LookRotation(normal));
        }
    }

    public static void PlayImpact(AudioClip clip, Vector3 point, float volume, float maxDistance)
    {
        if (clip == null) return;

        GameObject sound = new GameObject("Impact audio");
        sound.transform.position = point;
        AudioSource source = sound.AddComponent<AudioSource>();
        source.spatialBlend = 1f;
        source.rolloffMode = AudioRolloffMode.Linear;
        source.minDistance = 10f;
        source.maxDistance = maxDistance;
        source.dopplerLevel = 0f;
        source.PlayOneShot(clip, volume);
        Destroy(sound, clip.length + .1f);
    }

    public static bool FindHit(Vector3 origin, Vector3 direction, float distance, Transform ignoredTank, out RaycastHit closest)
    {
        closest = default;
        bool found = false;
        foreach (RaycastHit hit in Physics.RaycastAll(origin, direction, distance, Physics.DefaultRaycastLayers, QueryTriggerInteraction.Ignore))
        {
            if (ignoredTank != null && hit.collider.transform.IsChildOf(ignoredTank)) continue;
            if (found && hit.distance >= closest.distance) continue;
            closest = hit;
            found = true;
        }
        return found;
    }
}

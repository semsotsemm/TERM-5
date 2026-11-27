using UnityEngine;

public class TankShoot : MonoBehaviour
{
    public GameObject corePrefab;
    public float barrelLength = 9.0f;

    void Update()
    {
        if (Input.GetKeyDown(KeyCode.Space) && transform.root.GetComponent<bot_script>() == null)
        {
            Fire();
        }
    }

    public void Fire(bool firedByBot = false)
    {
        Transform owner = transform.root;
        Vector3 spawnPosition = transform.position + transform.forward * barrelLength;
        if (firedByBot && CoreMove.FindHit(transform.position, transform.forward, barrelLength, owner, out _))
        {
            spawnPosition = transform.position;
        }

        Quaternion spawnRotation = transform.rotation * Quaternion.Euler(90f, 0f, 0f);
        GameObject projectile = Instantiate(corePrefab, spawnPosition, spawnRotation);
        if (firedByBot)
        {
            projectile.GetComponent<CoreMove>().botOwner = owner;
            Physics.IgnoreCollision(projectile.GetComponent<Collider>(), owner.GetComponent<BoxCollider>());
        }

        AudioSource shot = GetComponent<AudioSource>();
        if (shot != null && shot.clip != null)
        {
            shot.PlayOneShot(shot.clip);
        }
    }
}

using UnityEngine;

public class ScriptScale : MonoBehaviour
{
    public float speed = 0.75f;

    private BoxCollider box;

    private void Start()
    {
        box = GetComponent<BoxCollider>();
    }

    private void FixedUpdate()
    {
        Vector3 scale = transform.localScale;
        scale.x = Mathf.Min(scale.x + speed * Time.fixedDeltaTime);
        scale.y = Mathf.Min(scale.y + speed * Time.fixedDeltaTime);
        transform.localScale = scale;

        Collider[] hits = Physics.OverlapBox(
            box.bounds.center,
            box.bounds.extents
        );

        foreach (Collider hit in hits)
        {
            if (hit == box)
                continue;

            Rigidbody body = hit.attachedRigidbody;

            if (body == null || body.isKinematic)
                continue;

            if (Physics.ComputePenetration(
                hit,
                hit.transform.position,
                hit.transform.rotation,
                box,
                box.transform.position,
                box.transform.rotation,
                out Vector3 direction,
                out float distance))
            {
                body.position += direction * (distance + 0.01f);
            }
        }
    }
}
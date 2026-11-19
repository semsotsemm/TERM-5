using UnityEngine;

public class CoreMove : MonoBehaviour
{
    public float speed = 160.0f;
    public float lifeTime = 3.0f;
    public float hitForce = 20.0f; // Сила толчка цели при попадании
    public GameObject explosion1;

    private bool hasHit = false;

    void Start()
    {
        Destroy(gameObject, lifeTime);
    }

    void Update()
    {
        if (hasHit) return;

        // Расстояние, на которое снаряд сдвинется за этот кадр
        float step = speed * Time.deltaTime;

        // Проверяем лучом отрезок пути впереди, чтобы снаряд не проскочил сквозь цель
        if (Physics.Raycast(transform.position, transform.up, out RaycastHit hit, step))
        {
            if (hit.collider.CompareTag("goal"))
            {
                HitGoal(hit.collider.gameObject, hit.point);
                return;
            }
        }

        transform.Translate(Vector3.up * step);
    }

    void OnCollisionEnter(Collision collision)
    {
        if (collision.gameObject.CompareTag("goal"))
        {
            HitGoal(collision.gameObject, transform.position);
        }
    }

    void HitGoal(GameObject target, Vector3 hitPoint)
    {
        if (hasHit) return;
        hasHit = true;

        GetComponent<Renderer>().enabled = false;
        GetComponent<Collider>().enabled = false;

        // Толкаем цель вручную, так как луч перехватывает попадание до физического удара
        Rigidbody targetRb = target.GetComponent<Rigidbody>();
        if (targetRb != null)
        {
            targetRb.AddForceAtPosition(transform.up * hitForce, hitPoint, ForceMode.Impulse);
        }

        if (explosion1 != null)
        {
            Instantiate(explosion1, hitPoint, Quaternion.identity);
        }
    }
}
using System.Collections;
using UnityEngine;

public class bot_script : MonoBehaviour
{
    public float moveSpeed = 2f;
    public float rotSpeedTank = 2f;
    public float rotSpeedBash = 3f;

    public Transform bash;
    public Transform stvol;
    public GameObject core;

    private bool canshoot = true;
    private TankShoot gun;
    private Rigidbody tankBody;

    void Start()
    {
        gun = stvol.GetComponent<TankShoot>();
        gun.corePrefab = core;
        tankBody = GetComponent<Rigidbody>();
    }

    void OnTriggerStay(Collider other)
    {
        if (!other.CompareTag("Player"))
        {
            return;
        }

        BoxCollider playerBody = other.GetComponent<BoxCollider>();
        if (playerBody == null)
        {
            return;
        }

        Vector3 direction = playerBody.bounds.center - transform.position;
        direction.y = 0f;
        float distance = direction.magnitude;

        float step = Time.fixedDeltaTime;
        bool moving = distance < 20f && distance > 16f;

        Quaternion bodyRotation = Quaternion.LookRotation(direction);
        if (moving)
        {
            tankBody.MoveRotation(Quaternion.Slerp(tankBody.rotation, bodyRotation, step * rotSpeedTank));
            tankBody.MovePosition(tankBody.position + direction.normalized * (moveSpeed * step));
        }

        Quaternion towerRotation = Quaternion.LookRotation(direction) * Quaternion.Euler(0f, 180f, 0f);
        bash.rotation = Quaternion.Slerp(bash.rotation, towerRotation, step * rotSpeedBash);

        Vector3 aim = playerBody.bounds.center - stvol.position;
        float pitch = Mathf.Atan2(-aim.y, new Vector2(aim.x, aim.z).magnitude) * Mathf.Rad2Deg;
        Quaternion barrelRotation = Quaternion.Euler(0f, 180f, 0f) * Quaternion.AngleAxis(Mathf.Clamp(pitch, -20f, 15f), Vector3.right);
        stvol.localRotation = Quaternion.Slerp(stvol.localRotation, barrelRotation, step * rotSpeedBash);

        if (canshoot && CoreMove.FindHit(stvol.position, stvol.forward, 40f, transform, out RaycastHit hit) && hit.collider == playerBody)
        {
            StartCoroutine(botshoot());
        }
    }

    IEnumerator botshoot()
    {
        canshoot = false;
        gun.Fire(true);
        yield return new WaitForSeconds(3f);
        canshoot = true;
    }
}

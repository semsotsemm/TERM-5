using System.Collections;
using UnityEngine;

public class bomb_dropper : MonoBehaviour
{
    public GameObject bomb_prefab;
    public Collider ground;
    public Material zone_material;
    public Vector2 tank_size = new Vector2(10f, 14f);
    public int bomb_count = 16;
    public float drop_height = 35f;
    public float drop_interval = 0.1f;
    public float cooldown = 3f;
    private float next_drop;

    void Update()
    {
        if (Input.GetKeyDown(KeyCode.Q)) DropBombs();
    }

    public void DropBombs()
    {
        if (bomb_prefab == null || ground == null || Time.time < next_drop) return;
        next_drop = Time.time + cooldown;
        StartCoroutine(Bombing());
    }

    IEnumerator Bombing()
    {
        Vector3 forward = Vector3.ProjectOnPlane(transform.forward, Vector3.up).normalized;
        Vector3 right = Vector3.Cross(Vector3.up, forward).normalized;
        Vector2 area_size = tank_size * 2f;
        Vector3 center = transform.position + forward * (tank_size.y * 1.5f);
        center.y = ground.bounds.max.y + 0.08f;
        ShowZone(center, forward, right, area_size);
        for (int i = 0; i < bomb_count; i++)
        {
            Vector3 position = center + right * Random.Range(-area_size.x / 2f, area_size.x / 2f)
                + forward * Random.Range(-area_size.y / 2f, area_size.y / 2f) + Vector3.up * drop_height;
            GameObject bomb = Instantiate(bomb_prefab, position, Quaternion.identity);
            bomb.GetComponent<bomb_script>().ground = ground;
            yield return new WaitForSeconds(drop_interval);
        }
    }

    void ShowZone(Vector3 center, Vector3 forward, Vector3 right, Vector2 size)
    {
        GameObject marker = new GameObject("Bombing area");
        LineRenderer line = marker.AddComponent<LineRenderer>();
        line.sharedMaterial = zone_material;
        line.loop = true;
        line.useWorldSpace = true;
        line.widthMultiplier = 0.2f;
        line.positionCount = 4;
        line.SetPositions(new[] {
            center - right * size.x / 2f - forward * size.y / 2f,
            center + right * size.x / 2f - forward * size.y / 2f,
            center + right * size.x / 2f + forward * size.y / 2f,
            center - right * size.x / 2f + forward * size.y / 2f
        });
        Destroy(marker, drop_height / 5f + bomb_count * drop_interval + 3f);
    }
}
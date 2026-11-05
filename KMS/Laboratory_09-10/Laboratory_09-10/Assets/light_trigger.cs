using System.Collections.Generic;
using UnityEngine;

public class light_trigger : MonoBehaviour
{
    public Light point_light;
    public float active_intensity = 10f;
    private readonly HashSet<Collider> tank_colliders = new HashSet<Collider>();
    private Transform tank_inside;
    private Vector3 initial_light_position;

    void Awake()
    {
        if (point_light != null)
        {
            initial_light_position = point_light.transform.position;
            point_light.intensity = 0f;
        }
    }

    void OnTriggerEnter(Collider other)
    {
        tank_script tank = other.GetComponentInParent<tank_script>();
        if (tank == null) return;
        tank_colliders.Add(other);
        tank_inside = tank.transform;
        if (point_light != null) point_light.intensity = active_intensity;
        FollowTank();
    }

    void OnTriggerExit(Collider other)
    {
        tank_colliders.Remove(other);
        if (tank_colliders.Count == 0) SwitchOff();
    }

    void LateUpdate()
    {
        FollowTank();
    }

    void FollowTank()
    {
        // Источник остаётся рядом с танком, чтобы вспышка была видна при Intensity = 10.
        if (tank_inside != null && point_light != null)
            point_light.transform.position = tank_inside.position + Vector3.up * 5f;
    }

    void SwitchOff()
    {
        tank_inside = null;
        if (point_light == null) return;
        point_light.intensity = 0f;
        point_light.transform.position = initial_light_position;
    }

    void OnDisable()
    {
        tank_colliders.Clear();
        SwitchOff();
    }
}

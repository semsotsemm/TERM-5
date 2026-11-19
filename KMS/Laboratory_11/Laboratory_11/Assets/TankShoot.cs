using UnityEngine;

public class TankShoot : MonoBehaviour
{
    // Ссылка на префаб снаряда (привяжем в Инспекторе)
    public GameObject corePrefab;

    // Длина ствола (расстояние от точки крепления ствола до его дула)
    public float barrelLength = 9.0f;

    void Update()
    {
        // Если нажали клавишу Пробел (один раз за нажатие)
        if (Input.GetKeyDown(KeyCode.Space))
        {
            // Точка появления = позиция ствола + смещение вперёд на длину ствола
            Vector3 spawnPosition = transform.position + transform.forward * barrelLength;

            Quaternion spawnRotation = transform.rotation * Quaternion.Euler(90f, 0f, 0f);
            Instantiate(corePrefab, spawnPosition, spawnRotation);
        }
    }
}
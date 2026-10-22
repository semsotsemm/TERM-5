using UnityEngine;
using UnityEngine.EventSystems;

public class CubeClick : MonoBehaviour, IPointerClickHandler
{
    public void OnPointerClick(PointerEventData event_data)
    {
        float red = Random.Range(0.0f, 1.0f);
        float green = Random.Range(0.0f, 1.0f);
        float blue = Random.Range(0.0f, 1.0f);

        Color new_color = new Color(red, green, blue);
        gameObject.GetComponent<Renderer>().material.color = new_color;

        Vector3 target = event_data.pointerPressRaycast.worldPosition;
        Vector3 collid = Camera.main.transform.position;

        int forse = 100;
        Vector3 distanse = (target - collid).normalized;
        collid = distanse * forse;

        gameObject.GetComponent<Rigidbody>().AddForceAtPosition(collid, target);
    }
}

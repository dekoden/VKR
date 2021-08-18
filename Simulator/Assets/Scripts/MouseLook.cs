using System.Collections;
using System.Collections.Generic;
using UnityEngine;

[AddComponentMenu("Control scripts/Mouse Look")]
public class MouseLook : MonoBehaviour
{
    public enum MouseMove
    {
        MouseXandY = 0,
        MouseX = 1,
        MouseY = 2,
    }
    public float sensitivityHor = 6.0f;
    public float sensitivityVer = 6.0f;
    public float minVertical = -45.0f;
    public float maxVertical = 45.0f;
    public MouseMove mouseMove = MouseMove.MouseXandY;

    private float rotationX = 0;
    // Start is called before the first frame update
    void Start()
    {
        Rigidbody body = GetComponent<Rigidbody>();
        if (body != null)
        {
            body.freezeRotation = true;
        }
        Cursor.lockState = CursorLockMode.Locked;
        Cursor.visible = false;
   }

    // Update is called once per frame
    void Update()
    {
        if (mouseMove == MouseMove.MouseX)
        {
            transform.Rotate(0, Input.GetAxis("Mouse X") * sensitivityHor, 0);
        }
        else if (mouseMove == MouseMove.MouseY)
        {
            rotationX -= Input.GetAxis("Mouse Y") * sensitivityVer;
            rotationX = Mathf.Clamp(rotationX, minVertical, maxVertical);
            float rotationY = transform.localEulerAngles.y;
            transform.localEulerAngles = new Vector3(rotationX, rotationY, 0);
        }
        else if (mouseMove == MouseMove.MouseXandY)
        {
            rotationX -= Input.GetAxis("Mouse Y") * sensitivityVer;
            rotationX = Mathf.Clamp(rotationX, minVertical, maxVertical);
            float rotationY = transform.localEulerAngles.y;
            rotationY += Input.GetAxis("Mouse X") * sensitivityHor;
            transform.localEulerAngles = new Vector3(rotationX, rotationY, 0);
        }
    }
}

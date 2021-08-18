using System.Collections;
using System.Collections.Generic;
using UnityEngine;

[RequireComponent(typeof(CharacterController))]
[AddComponentMenu("Control scripts/FPS Input")]
public class FPSInput : MonoBehaviour
{
    private CharacterController _characterController;
    public float speed = 6.0f;
    public float rotateSpeed = 20.0f;
    public float gravity = -9.8f;
    private float rotationY = 0;

    // Start is called before the first frame update
    void Start()
    {
        _characterController = GetComponent<CharacterController>();
    }

    // Update is called once per frame
    void Update()
    {
        rotationY += Input.GetAxis("Horizontal") * rotateSpeed * Time.deltaTime;
        transform.localEulerAngles = new Vector3(0, rotationY, 0);
        Vector3 motion = new Vector3(0, 0, Input.GetAxis("Vertical") * speed);
        motion = Vector3.ClampMagnitude(motion, speed);
        motion.y = gravity;
        motion *= Time.deltaTime;
        motion = transform.TransformDirection(motion);
        _characterController.Move(motion);

    }
}

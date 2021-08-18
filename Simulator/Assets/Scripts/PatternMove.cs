using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class PatternMove : MonoBehaviour
{
    [SerializeField] private VisionSystem visionSystem;
    [SerializeField] private float minAngle = -30f;
    [SerializeField] private float maxAngle = 30f;
    [SerializeField] private float minX = -0.15f;
    [SerializeField] private float maxX = 0.15f;
    [SerializeField] private float minY = 1.5f;
    [SerializeField] private float maxY = 3.7f;
    [SerializeField] private float minZ = -2.2f;
    [SerializeField] private float maxZ = -1.0f;
    // Start is called before the first frame update
    void Start()
    {
        
    }

    // Update is called once per frame
    void Update()
    {
        if (visionSystem.needCalibration && visionSystem.frameIndex < visionSystem.cameraCalibSamplesCount)
        {
            transform.localEulerAngles = new Vector3(
                UnityEngine.Random.Range(minAngle, maxAngle),
                UnityEngine.Random.Range(minAngle, maxAngle),
                UnityEngine.Random.Range(minAngle, maxAngle));
            transform.position = new Vector3(
                UnityEngine.Random.Range(minX, maxX),
                UnityEngine.Random.Range(minY, maxY),
                UnityEngine.Random.Range(minZ, maxZ));
        }
        else
        {
            Destroy(gameObject);
        }
    }
}

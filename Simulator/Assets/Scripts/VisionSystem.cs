using System;
using System.Collections;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using UnityEngine;
using UnityEngine.UI;

public class VisionSystem : MonoBehaviour
{
    public Camera camera1;
    public Camera camera2;
    public RenderTexture modifiedRender1;
    public RenderTexture modifiedRender2;
    public RenderTexture disparityMap;
    public int frameIndex = 0;
    public bool needCalibration = true;
    public int cameraCalibSamplesCount = 40;
    [SerializeField] private Slider numDisparitiesSlider;
    [SerializeField] private Slider blockSizeSlider;
    [SerializeField] private Slider alphaSlider;
    [SerializeField] private Text numDisparitiesText;
    [SerializeField] private Text blockSizeText;
    [SerializeField] private Text alphaText;

    private void ProcessCalibFrame(int cam)
    {
        //RenderTexToTex2D
        RenderTexture rt = (cam == 1) ? camera1.targetTexture : camera2.targetTexture;
        RenderTexture.active = rt;
        Texture2D tex = new Texture2D(rt.width, rt.height);
        tex.ReadPixels(new Rect(0, 0, tex.width, tex.height), 0, 0);
        Color32[] rawColor = tex.GetPixels32();

        //InitTexture
        Texture2D texNew = new Texture2D(tex.width, tex.height, TextureFormat.ARGB32, false);
        Color32[] pixel32 = texNew.GetPixels32();
        GCHandle pixelHandle = GCHandle.Alloc(pixel32, GCHandleType.Pinned);
        IntPtr pixelPtr = pixelHandle.AddrOfPinnedObject();
        unsafe
        {
            fixed (Color32* p1 = rawColor)
            {
                AddCalibSample((IntPtr)p1, pixelPtr, tex.width, tex.height, cam);
            }
        }

        //PointerToTexture2D
        texNew.SetPixels32(pixel32);
        texNew.Apply();

        //CopyTexToRender
        rt = (cam == 1) ? modifiedRender1 : modifiedRender2;
        RenderTexture.active = rt;
        Graphics.Blit(texNew, rt);

        //Free memory
        pixelHandle.Free();
        DestroyImmediate(tex);
        DestroyImmediate(texNew);
    }

    private void DepthMap()
    {
        //RenderTexToTex2D
        RenderTexture rtL = camera1.targetTexture;
        RenderTexture.active = rtL;
        Texture2D texL = new Texture2D(rtL.width, rtL.height);
        texL.ReadPixels(new Rect(0, 0, texL.width, texL.height), 0, 0);
        Color32[] rawColorL = texL.GetPixels32();

        RenderTexture rtR = camera2.targetTexture;
        RenderTexture.active = rtR;
        Texture2D texR = new Texture2D(rtR.width, rtR.height);
        texR.ReadPixels(new Rect(0, 0, texR.width, texR.height), 0, 0);
        Color32[] rawColorR = texR.GetPixels32();

        //InitTexture
        Texture2D texNew = new Texture2D(texL.width, texL.height, TextureFormat.ARGB32, false);
        Color32[] pixel32 = texNew.GetPixels32();
        GCHandle pixelHandle = GCHandle.Alloc(pixel32, GCHandleType.Pinned);
        IntPtr pixelPtr = pixelHandle.AddrOfPinnedObject();

        unsafe
        {
            fixed (Color32* pL = rawColorL, pR = rawColorR)
            {
                GetDepthMap((IntPtr)pL, (IntPtr)pR, pixelPtr, rtL.width, rtL.height);
            }
        }
                //PointerToTexture2D
        texNew.SetPixels32(pixel32);
        texNew.Apply();

        //CopyTexToRender
        RenderTexture rt = disparityMap;
        RenderTexture.active = rt;
        Graphics.Blit(texNew, rt);

        //Free memory
        pixelHandle.Free();
        DestroyImmediate(texL);
        DestroyImmediate(texR);
        DestroyImmediate(texNew);
    }

    private void SetNewParameters()
    {
        int numDisparities = 16 + (int)numDisparitiesSlider.value * 16;
        int blockSize = 7 + (int)blockSizeSlider.value * 2;
        double alpha = -1 + (int)alphaSlider.value * 0.1;
        numDisparitiesText.text = "NumDisparities: " + numDisparities;
        blockSizeText.text = "BlockSize: " + blockSize;
        alphaText.text = "Alpha: " + alpha;
        SetBMParameters(numDisparities, blockSize, alpha);
    }

    // Start is called before the first frame update
    void Start()
    {
        LoadParameters();
        numDisparitiesSlider.onValueChanged.AddListener(delegate { SetNewParameters(); });
        blockSizeSlider.onValueChanged.AddListener(delegate { SetNewParameters(); });
        alphaSlider.onValueChanged.AddListener(delegate { SetNewParameters(); });
    }

    // Update is called once per frame
    void Update()
    {
        if (needCalibration)
        {
            if (frameIndex == 0)
            {
                //Renderer[] renderers = GameObject.Find("Building").GetComponentsInChildren<MeshRenderer>();
                //foreach (Renderer r in renderers)
                //{
                //    r.enabled = !enabled;
                //}
            }
            if (frameIndex < cameraCalibSamplesCount)
            {
                ProcessCalibFrame(1);
                ProcessCalibFrame(2);
            }
            if (frameIndex == cameraCalibSamplesCount)
            {
                unsafe
                {
                    Calibrate(camera1.targetTexture.width, camera1.targetTexture.height);
                }
                needCalibration = false;
                //Renderer[] renderers = GameObject.Find("Building").GetComponentsInChildren<MeshRenderer>();
                //foreach (Renderer r in renderers)
                //{
                //    r.enabled = enabled;
                //}
            }
        }
        else
        {
            DepthMap();
        }
        frameIndex++;
    }

    [DllImport("Vision System", EntryPoint = "AddCalibSample")]
    unsafe static extern void AddCalibSample(IntPtr raw, IntPtr data, int width, int height, int cam);

    [DllImport("Vision System", EntryPoint = "Calibrate")]
    unsafe static extern void Calibrate(int width, int height);

    [DllImport("Vision System", EntryPoint = "GetDepthMap")]
    unsafe static extern void GetDepthMap(IntPtr rawL, IntPtr rawR, IntPtr data, int width, int height);

    [DllImport("Vision System", EntryPoint = "SetBMParameters")]
    static extern void SetBMParameters(int newNumDisparities, int newBlockSize, double newAlpha);

    [DllImport("Vision System", EntryPoint = "LoadParameters")]
    static extern void LoadParameters();
}

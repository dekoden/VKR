#ifndef __QuadrocopterBrain__Main__
#define __QuadrocopterBrain__Main__

struct Color32
{
	uchar r;
	uchar g;
	uchar b;
	uchar a;
};

extern "C"
{
	__declspec(dllexport) void AddCalibSample(Color32** raw, unsigned char* data, int width, int height, int cam);
	__declspec(dllexport) void Calibrate(int width, int height);
	__declspec(dllexport) void GetDepthMap(Color32** rawL, Color32** rawR, unsigned char* data, int width, int height);
	__declspec(dllexport) void SetBMParameters(int newNumDisparities, int newBlockSize, double newAlpha);
	__declspec(dllexport) void LoadParameters();
}

#endif /* defined(__QuadrocopterBrain__Main__) */

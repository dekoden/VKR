#include <opencv2/opencv.hpp>
#include "main.h"
#include <fstream>

using namespace cv;
using namespace std;

bool patternIsFound = false;
double rms1, rms2, rms;
int CHECKERBOARD[2]{ 6,9 };
int samplesCount = 0;
vector<Point3f> objp;
vector<vector<Point3f> > objectPoints;
vector<vector<Point2f> > imagePoints1, imagePoints2;
Mat cameraMatrix1, distCoeffs1, rotationMatrix1, T1, cameraMatrix2, distCoeffs2, rotationMatrix2, T2,
rotationMatrix, translationVector, essentialMatrix, fundamentalMatrix;
Size frameSize;
double alpha = 0.7; 
int numDisparities = 112, blockSize = 27;
Mat xyz;
bool selectObject = false;
Point origin; // Начальная точка щелчка мышью
Rect selection; // Определяем прямоугольную область

int saveDepth()
{
	cv::FileStorage fs("D:\\VKR\\Simulator\\depth.txt", cv::FileStorage::WRITE);
	fs << "image coordinates" << origin;
	fs << "world coordinates" << xyz.at<Vec3f>(origin);
}

static void onMouse(int event, int x, int y, int, void*)
{
	if (selectObject)
	{
		selection.x = MIN(x, origin.x);
		selection.y = MIN(y, origin.y);
		selection.width = std::abs(x - origin.x);
		selection.height = std::abs(y - origin.y);
	}

	switch (event)
	{
	case EVENT_LBUTTONDOWN: // Событие нажатия левой кнопки мыши
		origin = Point(x, y);
		selection = Rect(x, y, 0, 0);
		selectObject = true;
		saveDepth();
		/*cout << origin << "in world coordinate is: " << xyz.at<Vec3f>(origin) << endl;*/
		break;
	case EVENT_LBUTTONUP: // Событие, выпущенное левой кнопкой мыши
		selectObject = false;
		if (selection.width > 0 && selection.height > 0)
			break;
	}
}

int fillObjp()
{
	for (int k = 0; k < CHECKERBOARD[1]; k++)
	{
		for (int j = 0; j < CHECKERBOARD[0]; j++)
		{
			objp.push_back(cv::Point3f(j, k, 0));
		}
	}
	return 0;
}

int saveParams(string file_name)
{
	FileStorage fs(file_name, FileStorage::WRITE);
	if (!fs.isOpened())
	{
		std::cerr << "Can't open output file!" << std::endl;
		exit(1);
	}
	fs << "frameSize" << frameSize;
	fs << "samplesCount" << samplesCount;
	fs << "rms1" << rms1;
	fs << "cameraMatrix1" << cameraMatrix1;
	fs << "distCoeffs1" << distCoeffs1;
	fs << "rotationMatrix1" << rotationMatrix1;
	fs << "rms2" << rms2;
	fs << "cameraMatrix2" << cameraMatrix2;
	fs << "distCoeffs2" << distCoeffs2;
	fs << "rotationMatrix2" << rotationMatrix2;
	fs << "rms" << rms;
	fs << "rotationMatrix" << rotationMatrix;
	fs << "translationVector" << translationVector;
	fs << "essentialMatrix" << essentialMatrix;
	fs << "fundamentalMatrix" << fundamentalMatrix;

	return 0;
}

void LoadParameters()
{
	cv::FileStorage fs(/*"D:\\VKR\\Simulator\\output.txt"*/"D:\\НИР\\UnityProject\\New Unity Project\\output.txt", cv::FileStorage::READ);
	if (!fs.isOpened())
	{
		std::cout << "Failed to open ini parameters" << std::endl;
		exit(-1);
	}

	fs["frameSize"] >> frameSize;
	fs["cameraMatrix1"] >> cameraMatrix1;
	fs["distCoeffs1"] >> distCoeffs1;
	fs["rotationMatrix1"] >> rotationMatrix1;
	fs["cameraMatrix2"] >> cameraMatrix2;
	fs["distCoeffs2"] >> distCoeffs2;
	fs["rotationMatrix2"] >> rotationMatrix2;
	fs["rotationMatrix"] >> rotationMatrix;
	fs["translationVector"] >> translationVector;
	fs["essentialMatrix"] >> essentialMatrix;
	fs["fundamentalMatrix"] >> fundamentalMatrix;

	namedWindow("Depth map", WINDOW_FREERATIO);
	setMouseCallback("Depth map", onMouse, 0);
}

void AddCalibSample(Color32** raw, unsigned char* data, int width, int height, int cam)
{
	Mat frame = Mat(height, width, CV_8UC4, raw);
	cvtColor(frame, frame, cv::COLOR_BGRA2BGR);
	flip(frame, frame, 0);
	Mat gray;
	cvtColor(frame, gray, cv::COLOR_BGR2GRAY);
	vector<cv::Point2f> corner_pts;
	bool success = cv::findChessboardCorners(gray, cv::Size(CHECKERBOARD[0], CHECKERBOARD[1]), corner_pts, cv::CALIB_CB_ADAPTIVE_THRESH | cv::CALIB_CB_FAST_CHECK | cv::CALIB_CB_NORMALIZE_IMAGE);
	if (success)
	{
		cv::TermCriteria criteria(cv::TermCriteria::Type::EPS | cv::TermCriteria::Type::MAX_ITER, 30, 0.001);

		cv::cornerSubPix(gray, corner_pts, cv::Size(11, 11), cv::Size(-1, -1), criteria);


		cv::drawChessboardCorners(frame, cv::Size(CHECKERBOARD[0], CHECKERBOARD[1]), corner_pts, success);

		if (cam == 1)
		{
			imagePoints1.push_back(corner_pts);
			patternIsFound = true;
		}
		if (cam == 2 && patternIsFound)
		{
			imagePoints2.push_back(corner_pts);
			patternIsFound = false;
		}
	}
	else if (cam == 2 && patternIsFound)
	{
		imagePoints1.pop_back();
		patternIsFound = false;
	}
	cv::cvtColor(frame, frame, cv::COLOR_BGR2BGRA);
	flip(frame, frame, 0);
	memcpy(data, frame.data, frame.total() * frame.elemSize());
}

void Calibrate(int width, int height)
{
	samplesCount = imagePoints1.size();
	frameSize = Size(width, height);
	fillObjp();
	for (int i = 0; i < imagePoints1.size(); i++)
	{
		objectPoints.push_back(objp);
	}
	FileStorage fs("D:\\VKR\\Simulator\\debug.txt", FileStorage::WRITE);
	if (!fs.isOpened())
	{
		std::cerr << "Can't open output file!" << std::endl;
		exit(1);
	}
	fs << "frameSize" << frameSize;
	fs << "imagePoints1" << imagePoints1;
	fs << "imagePoints2" << imagePoints2;
	fs << "objectPoints" << objectPoints;
	rms1 = cv::calibrateCamera(objectPoints, imagePoints1, frameSize, cameraMatrix1, distCoeffs1, rotationMatrix1, T1);
	rms2 = cv::calibrateCamera(objectPoints, imagePoints2, frameSize, cameraMatrix2, distCoeffs2, rotationMatrix2, T2);
	rms = cv::stereoCalibrate
	(
		objectPoints, imagePoints1, imagePoints2,
		cameraMatrix1, distCoeffs1,
		cameraMatrix2, distCoeffs2,
		frameSize,
		rotationMatrix,
		translationVector,
		essentialMatrix,
		fundamentalMatrix,
		cv::CALIB_USE_INTRINSIC_GUESS /*| CALIB_FIX_INTRINSIC*/,
		cv::TermCriteria(cv::TermCriteria::COUNT + cv::TermCriteria::EPS, 100, 1e-5)
	);
	saveParams("D:\\VKR\\Simulator\\output.txt");
	objp.clear();
	objectPoints.clear();
	imagePoints1.clear();
	imagePoints2.clear();
}

void GetDepthMap(Color32** rawL, Color32** rawR, unsigned char* data, int width, int height)
{
	Mat imgL = Mat(height, width, CV_8UC4, rawL);
	Mat imgR = Mat(height, width, CV_8UC4, rawR);
	cvtColor(imgL, imgL, cv::COLOR_BGRA2BGR);
	cvtColor(imgR, imgR, cv::COLOR_BGRA2BGR);
	flip(imgL, imgL, 0);
	flip(imgR, imgR, 0);
	Mat image1recified, image2recified;
	Mat map11, map12, map21, map22;
	Rect roi1, roi2;
	Mat Q, R1, P1, R2, P2;
	stereoRectify(cameraMatrix1, distCoeffs1, cameraMatrix2, distCoeffs2, Size(width, height), rotationMatrix, translationVector, R1, R2, P1, P2, Q, CALIB_ZERO_DISPARITY, alpha, Size(width, height), &roi1, &roi2);
	initUndistortRectifyMap(cameraMatrix1, distCoeffs1, R1, P1, Size(width, height), CV_16SC2, map11, map12);
	initUndistortRectifyMap(cameraMatrix2, distCoeffs2, R2, P2, Size(width, height), CV_16SC2, map21, map22);
	remap(imgL, image1recified, map11, map12, INTER_LINEAR);
	remap(imgR, image2recified, map21, map22, INTER_LINEAR);
	Mat image1gray, image2gray;
	cvtColor(image1recified, image1gray, cv::COLOR_BGR2GRAY);
	cvtColor(image2recified, image2gray, cv::COLOR_BGR2GRAY);
	/*Ptr<StereoSGBM> sgbm = StereoSGBM::create(0, numDisparities, blockSize);*/
	Ptr<StereoBM> sbm = StereoBM::create(numDisparities, blockSize);
	sbm->setROI1(roi1);
	sbm->setROI2(roi2);
	Mat disp, disp8bit;
	/*sgbm->compute(image1gray, image2gray, disp);*/
	sbm->compute(image1gray, image2gray, disp);
	disp.convertTo(disp8bit, CV_8U, 255 / (numDisparities * 16.));
	reprojectImageTo3D(disp, xyz, Q, 3);
	xyz *= 2.85;
	imshow("Depth map", disp8bit);
	flip(disp8bit, disp8bit, 0);
	cvtColor(disp8bit, disp8bit, cv::COLOR_GRAY2BGRA, 4);
	memcpy(data, disp8bit.data, disp8bit.total() * disp8bit.elemSize());
}

void SetBMParameters(int newNumDisparities, int newBlockSize, double newAlpha)
{
	numDisparities = newNumDisparities;
	blockSize = newBlockSize;
	alpha = newAlpha;
}

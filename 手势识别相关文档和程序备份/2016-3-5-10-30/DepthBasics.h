//------------------------------------------------------------------------------
// <copyright file="DepthBasics.h" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
//------------------------------------------------------------------------------

#pragma once

#include "resource.h"
#include "ImageRenderer.h"
#include <iostream>
#include <vector>





#include "iostream"
#include "vector"
#include "OpenExr/include/ImathLine.h"
#include "OpenExr/include/ImathLineAlgo.h"
#include "OpenExr/include/ImathPlane.h"
#include "IDLPGesture.h"
using namespace IMATH_NAMESPACE;

#include <iostream>
#include <vector>


#include <fstream>
#include <iostream>
#include <vector>
#include "opencv/cv.h"
#include "opencv2/highgui/highgui.hpp"
#include "opencv/highgui.h"
#include "opencv/cv.h"
using namespace std;
using namespace cv;


#define MAX_HAND_NUM 5
#define foc 370 

class CDepthBasics
{
	static const int        cDepthWidth = 512;
	static const int        cDepthHeight = 424;


public:
	/// <summary>
	/// Constructor
	/// </summary>
	CDepthBasics();

	/// <summary>
	/// Destructor
	/// </summary>
	~CDepthBasics();

	/// <summary>
	/// Handles window messages, passes most to the class instance to handle
	/// </summary>
	/// <param name="hWnd">window message is for</param>
	/// <param name="uMsg">message</param>
	/// <param name="wParam">message data</param>
	/// <param name="lParam">additional message data</param>
	/// <returns>result of message processing</returns>
	static LRESULT CALLBACK MessageRouter(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	/// <summary>
	/// Handle windows messages for a class instance
	/// </summary>
	/// <param name="hWnd">window message is for</param>
	/// <param name="uMsg">message</param>
	/// <param name="wParam">message data</param>
	/// <param name="lParam">additional message data</param>
	/// <returns>result of message processing</returns>
	LRESULT CALLBACK        DlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	/// <summary>
	/// Creates the main window and begins processing
	/// </summary>
	/// <param name="hInstance"></param>
	/// <param name="nCmdShow"></param>
	int                                                   Run(HINSTANCE hInstance, int nCmdShow);



    std::vector<sAction>                                  m_hands;
    std::vector<USHORT>                                   depthData;
	INT64                                                 FrameTime;    //timestamp of frame//
	INT64                                                 m_nStartTime; 

	V3f                                                   pCurrentPos;

	std::vector<V3f>                                      vKinectPos;   //标记点的kinect坐标//
	std::vector<V3f>                                      sCreenPos;


	//利用像素点确定识别区域，使得识别的区域较小，可以改用Kinect坐标系下的x,y,z坐标进行确定//

	float                                                   iMaxLimitx;
	float                                                   iMaxLimity;
	float                                                   iMaxLimitz;
	float                                                   iMinLimitx;
	float                                                   iMinLimity;
	float                                                   iMinLimitz;



	int                                                   SeekHands(cv::Mat& cv16img, cv::Mat& cv8img);
	V3f                                                   getRatio(const V3f &a, const V3f &b, const V3f & c);
	V3f                                                   convertTo(const V3f& pos);
	V3f                                                   getKinectPos(const V3f & pos);
	V3f                                                   getPixelPos(const V3f & pos); //利用Kinect坐标获取像素点坐标//

	bool                                                  Precalc16_fast( cv::Mat& cv16img, cv::Mat& cv8img);

	bool                                                  IsBetweenTwoPlane(const V3f  & v1, const V3f  &v2, const V3f & v3);
	Plane3f                                               GetPlane(V3f  & v1, V3f & v2, V3f & v3, V3f & v4);
	bool                                                  IsInZone(const V3f  & pKinectPos);
	V3f                                                   getProjectPos(const V3f & sKinectPos);
	V3f                                                   getProjectPos(const V3f & sKinectPos, const Plane3f & plane);
	bool                                                  GetPositionLocation(const V3f & v1, const V3f & v2, const V3f & v3, const V3f & v4, const V3f & v5);

	bool                                                  GetHandPos(const Mat & cv16Img, const std::vector<Point>& contour, cv::Point3f  & hand);

	void										          CalcAve(const cv::Mat& cv16img, std::vector<Point>& contour, int  & maxdepth, int & mindepth, float  & avg);
	float												  CalcAveDepth(const cv::Mat& cv16img, std::vector<Point>& contour);

	void                                                  InitData(const std::vector<V3f> & vmarkscreenpos, const std::vector<V3f> & vmarkKinectPos, float h, float h1, float Radius);

	float                                                h1; //确定手势为Enter手势//


	void                                                 depthRevise(cv::Mat & cv16Img, std::vector<Point> & srcContours, std::vector<Point> & contours);    //对边缘的深度数据修正//

	bool                                                 isBorderOrOut(const cv::Mat & cv16Img, const std::vector<Point> & contours, const cv::Point & p);  //判断点是否在边界上//

	bool                                                 avgDepth(const cv::Point2f & p, const std::vector<Point> & srcContours,cv::Mat & cv16Img);  //给定某个像素点，取该像素点以及其八个方向，剔除掉边界点和外点，求该像素点的平均深度//

	cv::Point2f                                          getMaxDepth(const std::vector<float>  & vOridinalDepth);   //给定某个像素点，取该像素点以及其八个方向,获取其中有效点最大深度对应的序号和深度值//
	cv::Point2f                                          getMinDepth(const std::vector<float>  & vOridinalDepth);   //给定某个像素点，取该像素点以及其八个方向,获取其中有效点最小深度对应的序号和深度值//

	bool                                                 elimateBadDepth(const cv::Point2f  & pMaxDepth, const cv::Point2f  & pMinDepth, const float  & avgDepth, std::vector<float>  & vOridinalDepth); //排除掉坏点，即远离平均值的点//

	void                                                 getValidInnerPoint(const cv::Mat & cv16Img, const std::vector<Point> & srcContours, cv::Point2f & pCenter); //考虑到边界点的深度值不稳定，将边界点的邻域中的内部点替换掉边界点//

	std::vector<cv::Point2f>                             getNeighbour(const cv::Point2f & p);       //获取到某个点的邻域中的点//
	float                                                getAvgPixely(std::vector<cv::Point> & contours);


	//for test//
	void                                                 on_trackbar(int pos, std::vector<vector<Point>> & contours, cv::Mat & cv16img);

	float                                                fOutThreshold ;


	void                                                  setPixel(int markpixelx, int markpixely);

	void                                                 ConvertImagePointToHand(const V3f& img, sAction& act, const std::vector<sAction> preHands);  //用于获取手势的姿态//


	//test://
	//用于计算点到平面的距离//
	void                                                  CalDistFromPointToPlane(const V3f &p, const Plane3f & plane, float dist);
	//用于计算点到平面的距离//
	//test://

	//test//
	//讨论主持人在屏幕一侧的情形//
	bool                                                  getSummitPoint(const float  & threshold, const std::vector<cv::Point>  & srcContours, cv::Point  & fSummitPoint,const cv::Mat & cv16img);//若主持人在一侧，通过寻找极端的pixelx,pixely坐标得到手势尖端点的坐标//
	//讨论主持人在屏幕一侧的情形//
	//test//

	bool                                                  IsInOrOutOfZone(const V3f  & KinectPos, const float & fSpeedLength, V3f  vfPospre, V3f vfPosAfter);//判断点是否在边界上//

	int                                                   markpixelx;
	int                                                   markpixely;

	int                                                   iPointthreshold ;   //对于有可能出现的，找到的较大值点总在已经找到的边界轮廓附近的情况，使得后面的非需要的边界轮廓点出现在视野中，采取的方法为，设定点的阈值//
	int                                                   depthrevisethreshold;  //当该点的深度和平均值的深度差在阈值以内，我们认为该点的深度是可靠的，可以不需要对深度进行修正//
	float                                                 Radius;               //手势保持静止的半径阈值//
	float                                                 h;                    //手势活动的深度//

	Plane3f                                               pScreenPlane;          //屏幕平面//

	Plane3f                                               pdisScreenPlane;          //离屏平面//

	std::vector<V3f>                                      vdisMarkPoint;           //离屏平面的四个限制点//

	int                                                   identPixelx;
	int                                                   identPixely;


	//test//
	std::vector<int>                                      videntPixelx;
	std::vector<int>                                      videntPixely;


	//for revise depth//
	cv::Point                                             fSummitPoint;

	//for test//
	int                                                   identPixelx1;
	int                                                   identPixely1;


	int                                                   identPixelx2;
	int                                                   identPixely2;

	int                                                   identPixelx3;
	int                                                   identPixely3;
	//用于判定失去的手势的原因是离开识别区域或者是检测不到引起的HandAction的丢失，flag =true：表示离开识别区域;flag = false表示，由于点未检测到引起的//
	bool                                                  bIsInOrOutFlag;  

//	std::vector<bool>                                     vbIsInOrOutFlag;  //用于存储当前帧对应的手势是在边界点还是在离边界点较远的位置/


	float                                                 fActorOnOneSidethreshold; //主持人是否在一侧的阈值//

	cv::Point2f                                           m_CritMarkPoint;        //用于判定手势的运动状态的数据//

	//用于判断匹配的手势是否是同一个手势的阈值//
	std::vector<float>                                    vHandPosThreshold;
	float                                                 fHandPosThreshold;  //用于辅助测试,判断是否是同一个手势://
	//用于判断匹配的手势是否是同一个手势的阈值//

	std::vector <V3f>                                     vfKinectPos;    //为当前手势的KinectPos//
	

	//用于测试修正的深度是否适合//

	std::vector<cv::Point>                                identPixel;

	std::vector<std::vector<cv::Point3f>>                 vPos;                   //记录前两帧对应的手势的pos//


	//用于获取到当前帧数据与前一帧手势数据中的对应pos//
	bool                                                  getMarchLastHandPos(const std::vector<sAction>  & vLastHand, const cv::Point3f  & kinectPos, const float & threshold, int  & index);



	//for test//

	UINT                                                  getMaxDepth();
	UINT                                                  getMinDepth();

	UINT                                                  iMaxDepth;    //限制区域的最大深度//
	UINT                                                  iMinDepth;    //限制区域的最小深度//

	int                                                   minPixelx;   //屏幕边缘限制的最大最小像素//     //UINT与int类型运算得到结果也是UINT//
	int                                                   maxPixelx;   //屏幕边缘限制的最大最小像素//
	int                                                   minPixely;   //屏幕边缘限制的最大最小像素//
	int                                                   maxPixely;   //屏幕边缘限制的最大最小像素//

 	cv::Mat                                              cv16Img;
 	cv::Mat                                              cv8Img;

	//for test//





	//利用K-Means算法剔除掉选出的5个点中的4个得到我们所需要的指尖点//
	 cv::Point2f                                          getSummitPoint(const std::vector<cv::Point> & srcContours, const cv::Mat & cv16Img, std::vector<cv::Point> & contours);
	 std::vector<std::vector<cv::Point>>                  getGroup(const int & k, const std::vector<cv::Point>  & srcContours, const float  & threshold);
	 void                                                 UpdateGroup(const std::vector<cv::Point> & vMarkPoint, const std::vector<cv::Point>  & srcContours, std::vector<std::vector<cv::Point>> &vGroups);
	 float                                                getMin(std::vector<float> vDist);
	 cv::Point                                            getOptimalPoint(const std::vector<cv::Point> & srcContours);
	 float                                                getAvgDepth(const cv::Mat & cv16Img, std::vector<cv::Point> & contours);
	 //利用K-Means算法剔除掉选出的5个点中的4个得到我们所需要的指尖点//


private:
	HWND                    m_hWnd;
	
	INT64                   m_nLastCounter;
	double                  m_fFreq;
	INT64                   m_nNextStatusTime;
	DWORD                   m_nFramesSinceUpdate;
	bool                    m_bSaveScreenshot;

	// Current Kinect
	IKinectSensor*          m_pKinectSensor;

	// Depth reader
	IDepthFrameReader*      m_pDepthFrameReader;

	// Direct2D
	ImageRenderer*          m_pDrawDepth;
	ID2D1Factory*           m_pD2DFactory;
	RGBQUAD*                m_pDepthRGBX;

	/// <summary>
	/// Main processing function
	/// </summary>
	void                    Update();

	/// <summary>
	/// Initializes the default Kinect sensor
	/// </summary>
	/// <returns>S_OK on success, otherwise failure code</returns>
	HRESULT                 InitializeDefaultSensor();

	/// <summary>
	/// Handle new depth data
	/// <param name="nTime">timestamp of frame</param>
	/// <param name="pBuffer">pointer to frame data</param>
	/// <param name="nWidth">width (in pixels) of input image data</param>
	/// <param name="nHeight">height (in pixels) of input image data</param>
	/// <param name="nMinDepth">minimum reliable depth</param>
	/// <param name="nMaxDepth">maximum reliable depth</param>
	/// </summary>
	void                    ProcessDepth(INT64 nTime, const UINT16* pBuffer, int nHeight, int nWidth, USHORT nMinDepth, USHORT nMaxDepth);

	/// <summary>
	/// Set the status bar message
	/// </summary>
	/// <param name="szMessage">message to display</param>
	/// <param name="nShowTimeMsec">time in milliseconds for which to ignore future status messages</param>
	/// <param name="bForce">force status update</param>
	bool                    SetStatusMessage(_In_z_ WCHAR* szMessage, DWORD nShowTimeMsec, bool bForce);

	/// <summary>
	/// Get the name of the file where screenshot will be stored.
	/// </summary>
	/// <param name="lpszFilePath">string buffer that will receive screenshot file name.</param>
	/// <param name="nFilePathSize">number of characters in lpszFilePath string buffer.</param>
	/// <returns>
	/// S_OK on success, otherwise failure code.
	/// </returns>
	HRESULT                 GetScreenshotFileName(_Out_writes_z_(nFilePathSize) LPWSTR lpszFilePath, UINT nFilePathSize);

	/// <summary>
	/// Save passed in image data to disk as a bitmap
	/// </summary>
	/// <param name="pBitmapBits">image data to save</param>
	/// <param name="lWidth">width (in pixels) of input image data</param>
	/// <param name="lHeight">height (in pixels) of input image data</param>
	/// <param name="wBitsPerPixel">bits per pixel of image data</param>
	/// <param name="lpszFilePath">full file path to output bitmap to</param>
	/// <returns>indicates success or failure</returns>
	HRESULT                 SaveBitmapToFile(BYTE* pBitmapBits, LONG lWidth, LONG lHeight, WORD wBitsPerPixel, LPCWSTR lpszFilePath);
};


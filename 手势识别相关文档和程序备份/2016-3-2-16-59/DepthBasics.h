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
	int                     Run(HINSTANCE hInstance, int nCmdShow);



	std::vector<sAction>            m_hands;
	std::vector<USHORT>             depthData;
	INT64                           FrameTime;    //timestamp of frame//
	INT64                           m_nStartTime; 

	V3f                            pCurrentPos;

	std::vector<V3f>                vKinectPos;   //标记点的kinect坐标//
	std::vector<V3f>                sCreenPos;

	int                             SeekHands(const cv::Mat& cv16img, cv::Mat& cv8img);
	void                            ConvertImagePointToHand(const V3f& img, sAction& act);

	void                            ConvertImagePointToHand(const V3f& p, sAction& act, const std::vector<sAction> preHands);
	V3f                             getRatio(const V3f &a, const V3f &b, const V3f & c);
	V3f                             convertTo(const V3f& pos);
	V3f                             getKinectPos(const V3f & pos);

	bool                            Precalc16_fast(const cv::Mat& cv16img, cv::Mat& cv8img);

	bool                            IsBetweenTwoPlane(const V3f  & v1, const V3f  &v2, const V3f & v3);
	Plane3f                         GetPlane(V3f  & v1, V3f & v2, V3f & v3, V3f & v4);
	bool                            IsInZone(const V3f  & pKinectPos);
	V3f                             getProjectPos(const V3f & sKinectPos);
	bool                            GetPositionLocation(const V3f & v1, const V3f & v2, const V3f & v3, const V3f & v4, const V3f & v5);

	bool                            GetHandPos(const Mat & cv16Img, const std::vector<Point>& contour, cv::Point3f  & hand);

	void                            CalcAve(const cv::Mat& cv16img, std::vector<Point>& contour, int  & maxdepth, int & mindepth, float  & avg);
	float                           CalcAveDepth(const cv::Mat& cv16img, std::vector<Point>& contour);

	void                            InitData(const std::vector<V3f> & vmarkscreenpos, const std::vector<V3f> & vmarkKinectPos, float h, float h1, float Radius);

	bool                            avgDepth(const cv::Point2f & p, const std::vector<Point> & srcContours, cv::Mat & cv16Img);
	cv::Point2f                     getSummitPoint(const std::vector<cv::Point> & srcContours, const cv::Mat & cv16Img, std::vector<cv::Point> & contours);
	float                           h1; //确定手势为Enter手势//

	//新增加的到头文件里面的函数//
	int                             SeekHands(cv::Mat& cv16img, cv::Mat& cv8img);
	void                            on_trackbar(int pos, std::vector<vector<Point>> & contours, cv::Mat & cv16Img);
	bool                            getSummitPoint(const float  & threshold, const std::vector<cv::Point>  & srcContours, cv::Point  & fSummitPoint);
	void                            depthRevise(cv::Mat & cv16Img, std::vector<Point> & srcContours, std::vector<Point> & contours);
	void                            UpdateGroup(const std::vector<cv::Point> & vMarkPoint, const std::vector<cv::Point>  & srcContours, std::vector<std::vector<cv::Point>> &vGroups);
	float                           getMin(std::vector<float> vDist);
	bool                            elimateBadDepth(const cv::Point2f  & pMaxDepth, const cv::Point2f  & pMinDepth, const float  & avgDepth, std::vector<float>  & vOridinalDepth);
	float                           getAvgDepth(const cv::Mat & cv16Img, std::vector<cv::Point> & contours);
	std::vector<std::vector<cv::Point>>  getGroup(const int & k, const std::vector<cv::Point>  & srcContours, const float  & threshold);
	void                            CalDistFromPointToPlane(const V3f &p, const Plane3f & plane, float dist);
	bool                            getMarchLastHandPos(const std::vector<sAction>  & vLastHand, const cv::Point3f  & kinectPos, const float & threshold, int  & index);
	void                            setPixel(int markpixelx, int markpixely);
	cv::Point2f                     getMinDepth(const std::vector<float>  & vOridinalDepth);
	cv::Point2f                     getMaxDepth(const std::vector<float>  & vOridinalDepth);
	std::vector<cv::Point2f>        getNeighbour(const cv::Point2f & p);
	cv::Point                       getOptimalPoint(const std::vector<cv::Point> & srcContours);
	void                            getValidInnerPoint(const cv::Mat & cv16Img, const std::vector<Point> & srcContours, cv::Point2f & p);
	bool                            isBorderOrOut(const cv::Mat & cv16Img, const std::vector<Point> & contours, const cv::Point & p);
	bool                            IsInOrOutOfZone(const V3f  & KinectPos, const float & fSpeedLength, V3f  vfPospre, V3f vfPosAfter);
	bool                            Precalc16_fast(cv::Mat & cv16img, cv::Mat& cv8img);


	


	float                            Radius;               //手势保持静止的半径阈值//
	float                            h;                    //手势活动的深度//

	Plane3f                          pScreenPlane;          //屏幕平面//

	Plane3f                          pdisScreenPlane;          //离屏平面//

	std::vector<V3f>                vdisMarkPoint;           //离屏平面的四个限制点//

	UINT                            getMaxDepth();
	UINT                            getMinDepth();

	UINT                            iMaxDepth;
	UINT                            iMinDepth;

	UINT                            minPixelx;
	UINT                            maxPixelx;
	UINT                            minPixely;
	UINT                            maxPixely;

	//for test//


	//新增加的头文件的变量//
	float                           fHandPosThreshold ;

	float                           depthrevisethreshold ;

	float                           fOutThreshold ;

	float                           iPointthreshold;

	float                           fActorOnOneSidethreshold ;  //主持人是否在一侧的阈值//

	bool                            bIsInOrOutFlag;

	float                           identPixelx;
    float                           identPixely ;
	float                           identPixelx1;
	float                           identPixely1 ;
	float                           identPixelx2;
	float                           identPixely2 ;
	float                           identPixelx3;
	float                           identPixely3;
	float                           markpixelx;
	float                           markpixely;

	std::vector<V3f>                vfKinectPos;

	std::vector<cv::Point>          identPixel;
	std::vector<std::vector<cv::Point3f>>    vPos;
	std::vector<float>              vHandPosThreshold;



	



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


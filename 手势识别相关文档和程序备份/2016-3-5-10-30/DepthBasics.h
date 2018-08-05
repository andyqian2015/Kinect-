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

	std::vector<V3f>                                      vKinectPos;   //��ǵ��kinect����//
	std::vector<V3f>                                      sCreenPos;


	//�������ص�ȷ��ʶ������ʹ��ʶ��������С�����Ը���Kinect����ϵ�µ�x,y,z�������ȷ��//

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
	V3f                                                   getPixelPos(const V3f & pos); //����Kinect�����ȡ���ص�����//

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

	float                                                h1; //ȷ������ΪEnter����//


	void                                                 depthRevise(cv::Mat & cv16Img, std::vector<Point> & srcContours, std::vector<Point> & contours);    //�Ա�Ե�������������//

	bool                                                 isBorderOrOut(const cv::Mat & cv16Img, const std::vector<Point> & contours, const cv::Point & p);  //�жϵ��Ƿ��ڱ߽���//

	bool                                                 avgDepth(const cv::Point2f & p, const std::vector<Point> & srcContours,cv::Mat & cv16Img);  //����ĳ�����ص㣬ȡ�����ص��Լ���˸������޳����߽�����㣬������ص��ƽ�����//

	cv::Point2f                                          getMaxDepth(const std::vector<float>  & vOridinalDepth);   //����ĳ�����ص㣬ȡ�����ص��Լ���˸�����,��ȡ������Ч�������ȶ�Ӧ����ź����ֵ//
	cv::Point2f                                          getMinDepth(const std::vector<float>  & vOridinalDepth);   //����ĳ�����ص㣬ȡ�����ص��Լ���˸�����,��ȡ������Ч����С��ȶ�Ӧ����ź����ֵ//

	bool                                                 elimateBadDepth(const cv::Point2f  & pMaxDepth, const cv::Point2f  & pMinDepth, const float  & avgDepth, std::vector<float>  & vOridinalDepth); //�ų������㣬��Զ��ƽ��ֵ�ĵ�//

	void                                                 getValidInnerPoint(const cv::Mat & cv16Img, const std::vector<Point> & srcContours, cv::Point2f & pCenter); //���ǵ��߽������ֵ���ȶ������߽��������е��ڲ����滻���߽��//

	std::vector<cv::Point2f>                             getNeighbour(const cv::Point2f & p);       //��ȡ��ĳ����������еĵ�//
	float                                                getAvgPixely(std::vector<cv::Point> & contours);


	//for test//
	void                                                 on_trackbar(int pos, std::vector<vector<Point>> & contours, cv::Mat & cv16img);

	float                                                fOutThreshold ;


	void                                                  setPixel(int markpixelx, int markpixely);

	void                                                 ConvertImagePointToHand(const V3f& img, sAction& act, const std::vector<sAction> preHands);  //���ڻ�ȡ���Ƶ���̬//


	//test://
	//���ڼ���㵽ƽ��ľ���//
	void                                                  CalDistFromPointToPlane(const V3f &p, const Plane3f & plane, float dist);
	//���ڼ���㵽ƽ��ľ���//
	//test://

	//test//
	//��������������Ļһ�������//
	bool                                                  getSummitPoint(const float  & threshold, const std::vector<cv::Point>  & srcContours, cv::Point  & fSummitPoint,const cv::Mat & cv16img);//����������һ�࣬ͨ��Ѱ�Ҽ��˵�pixelx,pixely����õ����Ƽ�˵������//
	//��������������Ļһ�������//
	//test//

	bool                                                  IsInOrOutOfZone(const V3f  & KinectPos, const float & fSpeedLength, V3f  vfPospre, V3f vfPosAfter);//�жϵ��Ƿ��ڱ߽���//

	int                                                   markpixelx;
	int                                                   markpixely;

	int                                                   iPointthreshold ;   //�����п��ܳ��ֵģ��ҵ��Ľϴ�ֵ�������Ѿ��ҵ��ı߽����������������ʹ�ú���ķ���Ҫ�ı߽��������������Ұ�У���ȡ�ķ���Ϊ���趨�����ֵ//
	int                                                   depthrevisethreshold;  //���õ����Ⱥ�ƽ��ֵ����Ȳ�����ֵ���ڣ�������Ϊ�õ������ǿɿ��ģ����Բ���Ҫ����Ƚ�������//
	float                                                 Radius;               //���Ʊ��־�ֹ�İ뾶��ֵ//
	float                                                 h;                    //���ƻ�����//

	Plane3f                                               pScreenPlane;          //��Ļƽ��//

	Plane3f                                               pdisScreenPlane;          //����ƽ��//

	std::vector<V3f>                                      vdisMarkPoint;           //����ƽ����ĸ����Ƶ�//

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
	//�����ж�ʧȥ�����Ƶ�ԭ�����뿪ʶ����������Ǽ�ⲻ�������HandAction�Ķ�ʧ��flag =true����ʾ�뿪ʶ������;flag = false��ʾ�����ڵ�δ��⵽�����//
	bool                                                  bIsInOrOutFlag;  

//	std::vector<bool>                                     vbIsInOrOutFlag;  //���ڴ洢��ǰ֡��Ӧ���������ڱ߽�㻹������߽���Զ��λ��/


	float                                                 fActorOnOneSidethreshold; //�������Ƿ���һ�����ֵ//

	cv::Point2f                                           m_CritMarkPoint;        //�����ж����Ƶ��˶�״̬������//

	//�����ж�ƥ��������Ƿ���ͬһ�����Ƶ���ֵ//
	std::vector<float>                                    vHandPosThreshold;
	float                                                 fHandPosThreshold;  //���ڸ�������,�ж��Ƿ���ͬһ������://
	//�����ж�ƥ��������Ƿ���ͬһ�����Ƶ���ֵ//

	std::vector <V3f>                                     vfKinectPos;    //Ϊ��ǰ���Ƶ�KinectPos//
	

	//���ڲ�������������Ƿ��ʺ�//

	std::vector<cv::Point>                                identPixel;

	std::vector<std::vector<cv::Point3f>>                 vPos;                   //��¼ǰ��֡��Ӧ�����Ƶ�pos//


	//���ڻ�ȡ����ǰ֡������ǰһ֡���������еĶ�Ӧpos//
	bool                                                  getMarchLastHandPos(const std::vector<sAction>  & vLastHand, const cv::Point3f  & kinectPos, const float & threshold, int  & index);



	//for test//

	UINT                                                  getMaxDepth();
	UINT                                                  getMinDepth();

	UINT                                                  iMaxDepth;    //���������������//
	UINT                                                  iMinDepth;    //�����������С���//

	int                                                   minPixelx;   //��Ļ��Ե���Ƶ������С����//     //UINT��int��������õ����Ҳ��UINT//
	int                                                   maxPixelx;   //��Ļ��Ե���Ƶ������С����//
	int                                                   minPixely;   //��Ļ��Ե���Ƶ������С����//
	int                                                   maxPixely;   //��Ļ��Ե���Ƶ������С����//

 	cv::Mat                                              cv16Img;
 	cv::Mat                                              cv8Img;

	//for test//





	//����K-Means�㷨�޳���ѡ����5�����е�4���õ���������Ҫ��ָ���//
	 cv::Point2f                                          getSummitPoint(const std::vector<cv::Point> & srcContours, const cv::Mat & cv16Img, std::vector<cv::Point> & contours);
	 std::vector<std::vector<cv::Point>>                  getGroup(const int & k, const std::vector<cv::Point>  & srcContours, const float  & threshold);
	 void                                                 UpdateGroup(const std::vector<cv::Point> & vMarkPoint, const std::vector<cv::Point>  & srcContours, std::vector<std::vector<cv::Point>> &vGroups);
	 float                                                getMin(std::vector<float> vDist);
	 cv::Point                                            getOptimalPoint(const std::vector<cv::Point> & srcContours);
	 float                                                getAvgDepth(const cv::Mat & cv16Img, std::vector<cv::Point> & contours);
	 //����K-Means�㷨�޳���ѡ����5�����е�4���õ���������Ҫ��ָ���//


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


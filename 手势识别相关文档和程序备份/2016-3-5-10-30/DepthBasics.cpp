//------------------------------------------------------------------------------
// <copyright file="DepthBasics.cpp" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
//------------------------------------------------------------------------------
#include "atlbase.h"
#include "stdafx.h"
#include <strsafe.h>
#include "resource.h"
#include "DepthBasics.h"
#include <stdio.h>
#include <vector>
#include <iostream>
#include <math.h>
#include <limits>
#include <Wincodec.h>

#include <iostream>
#include <vector>

#include <fstream>


#include "stdio.h"
#include "stdlib.h"
#include "time.h"

#include "opencv/cv.h"
#include "opencv2/highgui/highgui.hpp"
#include "opencv/highgui.h"
using namespace std;
using namespace cv;



SHORT max_depth_regin[25650];
SHORT min_depth_regin[25];

IplImage *g_pGrayImage = NULL;
IplImage *g_pBinaryImage = NULL;

float depthPixelx = 0;
float depthPixely = 0;

float  max_depth_cursor = 0;
float  min_depth_cursor = 10000;

int  useTime = 0;
int useTime1 = 0;
int useTime2 = 0;
int useTime3 = 0;
int useTime4 = 0;
int  start   = 0;
int endTime =  0;
int  endtime1 = 0;
int endTime2 = 0;
int endTime3 = 0;
int endTime4 = 0;


bool printflag = false;

/// <summary>
/// Entry point for the application
/// </summary>
/// <param name="hInstance">handle to the application instance</param>
/// <param name="hPrevInstance">always 0</param>
/// <param name="lpCmdLine">command line arguments</param>
/// <param name="nCmdShow">whether to display minimized, maximized, or normally</param>
/// <returns>status</returns>



/// <summary>
/// Constructor
/// </summary>
CDepthBasics::CDepthBasics() :
m_hWnd(NULL),
m_nStartTime(0),
m_nLastCounter(0),
m_nFramesSinceUpdate(0),
m_fFreq(0),
m_nNextStatusTime(0LL),
m_bSaveScreenshot(false),
m_pKinectSensor(NULL),
m_pDepthFrameReader(NULL),
m_pD2DFactory(NULL),
m_pDrawDepth(NULL),
m_pDepthRGBX(NULL)
{
	LARGE_INTEGER qpf = { 0 };
	if (QueryPerformanceFrequency(&qpf))
	{
		m_fFreq = double(qpf.QuadPart);
	}

	// create heap storage for depth pixel data in RGBX format
	m_pDepthRGBX = new RGBQUAD[cDepthWidth * cDepthHeight];


	pCurrentPos = V3f(0, 0, 0);
	

}


/// <summary>
/// Destructor
/// </summary>
CDepthBasics::~CDepthBasics()
{
	// clean up Direct2D renderer
	if (m_pDrawDepth)
	{
		delete m_pDrawDepth;
		m_pDrawDepth = NULL;
	}

	if (m_pDepthRGBX)
	{
		delete[] m_pDepthRGBX;
		m_pDepthRGBX = NULL;
	}

	// clean up Direct2D
	SafeRelease(m_pD2DFactory);

	// done with depth frame reader
	SafeRelease(m_pDepthFrameReader);

	// close the Kinect Sensor
	if (m_pKinectSensor)
	{
		m_pKinectSensor->Close();
	}

	SafeRelease(m_pKinectSensor);
}

/// <summary>
/// Creates the main window and begins processing
/// </summary>
/// <param name="hInstance">handle to the application instance</param>
/// <param name="nCmdShow">whether to display minimized, maximized, or normally</param>
int CDepthBasics::Run(HINSTANCE hInstance, int nCmdShow)
{
	MSG       msg = { 0 };
	WNDCLASS  wc;

	// Dialog custom window class
	ZeroMemory(&wc, sizeof(wc));
	wc.style = CS_HREDRAW | CS_VREDRAW; 
	wc.cbWndExtra = DLGWINDOWEXTRA;
	wc.hCursor = LoadCursorW(NULL, IDC_ARROW);
	wc.hIcon = LoadIconW(hInstance, MAKEINTRESOURCE(IDI_APP));
	wc.lpfnWndProc = DefDlgProcW;
	wc.lpszClassName = L"DepthBasicsAppDlgWndClass";

	if (!RegisterClassW(&wc))
	{
		return 0;
	}

	// Create main application window
	HWND hWndApp = CreateDialogParamW(
		NULL,
		MAKEINTRESOURCE(IDD_APP),
		NULL,
		(DLGPROC)CDepthBasics::MessageRouter,
		reinterpret_cast<LPARAM>(this));

	// Show window
	ShowWindow(hWndApp, nCmdShow);
	
	// Main message loop
	while (WM_QUIT != msg.message)
	{

		start = clock();
		Update();

		int mode = 0;
		if (mode == 0) {
			int k = 0;
			if (depthData.back()== 1) {


				int t_ms = (FrameTime - m_nStartTime) / 1000;

				//for test check if the depth is right//

				for (int i = 0; i < cDepthHeight; i++) {
					for (int j = 0; j < cDepthWidth; j++) {
						if (depthData[k] != 0)
							cv16Img.ptr<USHORT>(i)[j] = depthData[k];
						k++;
					}
				}

				//for test check if the depth is right//
				k = 0;


				endtime1 = clock();
				useTime1 = endtime1 - start;
				Precalc16_fast(cv16Img, cv8Img);
				endTime2 = clock();
				useTime2 = endTime2 - endtime1;
				SeekHands(cv16Img, cv8Img);
				endTime3 = clock();
				useTime3 = endTime3 - endTime2;
 				
			

				/*
				//test//
				//��ȡ�Ա߽�ü����ͼ�����ֵ//
				std::fstream out8("D:\\NonZeroDepth.txt", std::ios::app);
				for (int i = 0; i < 424; i++) {
				for (int j = 0; j < 512; j++) {
				if (cv16Img.at<USHORT>(i, j) > 0) {
				out8 << j << " " <<i<< " " << cv16Img.at<USHORT>(i, j) << std::endl;
				}
				}
				}
				out8 << std::endl;
				out8 << std::endl;
				//��ȡ�Ա߽�ü����ͼ�����ֵ//
				//test//
				*/
//				depthData.clear();  //delete for contemporary 
				depthData[218000] = 0;	
				endTime = clock();
				useTime = (endTime - start);
			}
		}

		//get HandContour//

		//transform to opencv format//
		 
		while (PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE))
		{
			// If a dialog message will be taken care of by the dialog proc
			if (hWndApp && IsDialogMessageW(hWndApp, &msg))
			{
				continue;
			}

			TranslateMessage(&msg);
			DispatchMessageW(&msg);
		}

		endTime4 = clock();
		useTime4 = endTime4 - endTime3;
	}

	return static_cast<int>(msg.wParam);
}

/// <summary>
/// Main processing function
/// </summary>
void CDepthBasics::Update()
{
	if (!m_pDepthFrameReader)
	{
		return;
	}

	IDepthFrame* pDepthFrame = NULL;

	HRESULT hr = m_pDepthFrameReader->AcquireLatestFrame(&pDepthFrame);


	

	if (SUCCEEDED(hr))
	{
		INT64 nTime = 0;
		IFrameDescription* pFrameDescription = NULL;
		int nWidth = 0;
		int nHeight = 0;
		USHORT nDepthMinReliableDistance = 0;
		USHORT nDepthMaxDistance = 0;
		UINT nBufferSize = 0;
		UINT16 *pBuffer = NULL;

		hr = pDepthFrame->get_RelativeTime(&nTime);

		FrameTime = nTime;

		if (SUCCEEDED(hr))
		{
			hr = pDepthFrame->get_FrameDescription(&pFrameDescription);

		}

		if (SUCCEEDED(hr))
		{
			hr = pFrameDescription->get_Width(&nWidth);
		}

		if (SUCCEEDED(hr))
		{
			hr = pFrameDescription->get_Height(&nHeight);
		}

		if (SUCCEEDED(hr))
		{
			hr = pDepthFrame->get_DepthMinReliableDistance(&nDepthMinReliableDistance);
		}

		if (SUCCEEDED(hr))
		{
			// In order to see the full range of depth (including the less reliable far field depth)
			// we are setting nDepthMaxDistance to the extreme potential depth threshold
			nDepthMaxDistance = USHRT_MAX;

			// Note:  If you wish to filter by reliable depth distance, uncomment the following line.
			hr = pDepthFrame->get_DepthMaxReliableDistance(&nDepthMaxDistance);
		}

		if (SUCCEEDED(hr))
		{
			hr = pDepthFrame->AccessUnderlyingBuffer(&nBufferSize, &pBuffer);

		}

		if (SUCCEEDED(hr))
		{
			ProcessDepth(nTime, pBuffer, nWidth, nHeight, nDepthMinReliableDistance, nDepthMaxDistance);
		}

		SafeRelease(pFrameDescription);
	}

	SafeRelease(pDepthFrame);

}

/// <summary>
/// Handles window messages, passes most to the class instance to handle
/// </summary>
/// <param name="hWnd">window message is for</param>
/// <param name="uMsg">message</param>
/// <param name="wParam">message data</param>
/// <param name="lParam">additional message data</param>
/// <returns>result of message processing</returns>
LRESULT CALLBACK CDepthBasics::MessageRouter(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	CDepthBasics* pThis = NULL;

	if (WM_INITDIALOG == uMsg)
	{
		pThis = reinterpret_cast<CDepthBasics*>(lParam);
		SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pThis));
	}
	else
	{
		pThis = reinterpret_cast<CDepthBasics*>(::GetWindowLongPtr(hWnd, GWLP_USERDATA));
	}

	if (pThis)
	{
		return pThis->DlgProc(hWnd, uMsg, wParam, lParam);
	}

	return 0;
}

/// <summary>
/// Handle windows messages for the class instance
/// </summary>
/// <param name="hWnd">window message is for</param>
/// <param name="uMsg">message</param>
/// <param name="wParam">message data</param>
/// <param name="lParam">additional message data</param>
/// <returns>result of message processing</returns>
LRESULT CALLBACK CDepthBasics::DlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam){
	UNREFERENCED_PARAMETER(wParam);
	UNREFERENCED_PARAMETER(lParam);

	switch (message)
	{
	case WM_INITDIALOG:
	{
		// Bind application window handle
		m_hWnd = hWnd;

		// Init Direct2D
		D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &m_pD2DFactory);

		// Create and initialize a new Direct2D image renderer (take a look at ImageRenderer.h)
		// We'll use this to draw the data we receive from the Kinect to the screen
		m_pDrawDepth = new ImageRenderer();
		HRESULT hr = m_pDrawDepth->Initialize(GetDlgItem(m_hWnd, IDC_VIDEOVIEW), m_pD2DFactory, cDepthWidth, cDepthHeight, cDepthWidth * sizeof(RGBQUAD));
		if (FAILED(hr))
		{
			SetStatusMessage(L"Failed to initialize the Direct2D draw device.", 10000, true);
		}

		// Get and initialize the default Kinect sensor
		InitializeDefaultSensor();
	}
	break;

	// If the titlebar X is clicked, destroy app
	case WM_CLOSE:
		DestroyWindow(hWnd);
		break;

	case WM_DESTROY:
		// Quit the main message pump
		PostQuitMessage(0);
		break;

	case WM_LBUTTONDOWN: {
		max_depth_cursor = 0;
		min_depth_cursor = 10000;
		POINT lpPoint;
		int x = 0, y = 0;
		GetCursorPos(&lpPoint);
		ScreenToClient(m_hWnd, &lpPoint);
		x = lpPoint.x;
		y = lpPoint.y;


		depthPixelx = 512 * x / 768;
		depthPixely = 424 * y / 688;

		break;
	}

				 

 // Handle button press
	case WM_COMMAND:
		// If it was for the screenshot control and a button clicked event, save a screenshot next frame 

		if (IDC_BUTTON_SCREENSHOT == LOWORD(wParam) && BN_CLICKED == HIWORD(wParam))

		{
			m_bSaveScreenshot = true;
		}
		break;
	}

	return FALSE;
}

/// <summary>
/// Initializes the default Kinect sensor
/// </summary>
/// <returns>indicates success or failure</returns>
HRESULT CDepthBasics::InitializeDefaultSensor()
{
	HRESULT hr;

	hr = GetDefaultKinectSensor(&m_pKinectSensor);
	if (FAILED(hr))
	{
		return hr;
	}

	if (m_pKinectSensor)
	{
		// Initialize the Kinect and get the depth reader
		IDepthFrameSource* pDepthFrameSource = NULL;

		hr = m_pKinectSensor->Open();

		if (SUCCEEDED(hr))
		{
			hr = m_pKinectSensor->get_DepthFrameSource(&pDepthFrameSource);
		}

		if (SUCCEEDED(hr))
		{
			hr = pDepthFrameSource->OpenReader(&m_pDepthFrameReader);
		}

		SafeRelease(pDepthFrameSource);
	}

	if (!m_pKinectSensor || FAILED(hr))
	{
		SetStatusMessage(L"No ready Kinect found!", 10000, true);

		return E_FAIL;
	}

	return hr;
}

/// <summary>
/// Handle new depth data
/// <param name="nTime">timestamp of frame</param>
/// <param name="pBuffer">pointer to frame data</param>
/// <param name="nWidth">width (in pixels) of input image data</param>
/// <param name="nHeight">height (in pixels) of input image data</param>
/// <param name="nMinDepth">minimum reliable depth</param>
/// <param name="nMaxDepth">maximum reliable depth</param>
/// </summary>


void CDepthBasics::InitData(const std::vector<V3f> & vmarkscreenpos, const std::vector<V3f> & vmarkKinectPos, float h, float h1, float Radius) {


	m_CritMarkPoint = cv::Point2f(0, 0);

	//Pictute Mat init//
	cv16Img = cv::Mat(cDepthHeight, cDepthWidth, CV_16UC1);
	cv8Img =  cv::Mat(cDepthHeight, cDepthWidth, CV_8UC1);

	int m = cv16Img.at<USHORT>(2, 2);

	for (int i = 0; i < cDepthHeight; i++) {
		for (int j = 0; j < cDepthWidth; j++) {
			cv8Img.at<uchar>(i, j) = 0;
			cv16Img.at<USHORT>(i, j) = 0;
		}
	}

	
	int n = cv16Img.ptr<USHORT>(2)[2];

	depthData.resize(218001);   //for test//
	fSummitPoint = cv::Point(0, 0); //�����Ƽ�˵㸳��ֵ//

	//�������˶����ٶȳ�ʼ��//
	fHandPosThreshold = 200;

	depthrevisethreshold = 80;

	fOutThreshold = 10;

	iPointthreshold = 30;

	fActorOnOneSidethreshold = 10;  //�������Ƿ���һ�����ֵ//

	bIsInOrOutFlag = true;

	identPixelx = identPixely = 0;
	identPixelx1 = identPixely1 = 0;
	identPixelx2 = identPixely2 = 0;
	identPixelx3 = identPixely3 = 0;


	if (min(vmarkKinectPos[0].x, vmarkKinectPos[1].x) > min(vmarkKinectPos[2].x, vmarkKinectPos[3].x)) {
		minPixelx = max(vmarkKinectPos[2].x, vmarkKinectPos[3].x);
	}

	else  {
		minPixelx = max(vmarkKinectPos[0].x, vmarkKinectPos[1].x);
	}
//	minPixelx = min(min(vmarkKinectPos[0].x, vmarkKinectPos[1].x), min(vmarkKinectPos[2].x, vmarkKinectPos[3].x));

	if (min(vmarkKinectPos[0].y, vmarkKinectPos[3].y) > min(vmarkKinectPos[2].y, vmarkKinectPos[1].y)) {
		minPixely = max(vmarkKinectPos[1].y, vmarkKinectPos[2].y);
	}

	else {
		minPixely = max(vmarkKinectPos[0].y, vmarkKinectPos[3].y);
	}

	if (max(vmarkKinectPos[0].x, vmarkKinectPos[1].x) > max(vmarkKinectPos[2].x, vmarkKinectPos[3].x)) {
		maxPixelx = min(vmarkKinectPos[0].x, vmarkKinectPos[1].x);
	}

	else {
		maxPixelx = min(vmarkKinectPos[2].x, vmarkKinectPos[3].x);
	}

	if (max(vmarkKinectPos[0].y, vmarkKinectPos[3].y) > max(vmarkKinectPos[1].y, vmarkKinectPos[2].y)) {
		maxPixely = min(vmarkKinectPos[0].y, vmarkKinectPos[3].y);
	}

	else {
		maxPixely = min(vmarkKinectPos[1].y ,vmarkKinectPos[2].y);
	}


//	minPixely = min(min(vmarkKinectPos[0].y, vmarkKinectPos[1].y), min(vmarkKinectPos[2].y, vmarkKinectPos[3].y));

//	maxPixelx = max(max(vmarkKinectPos[0].x, vmarkKinectPos[1].x), max(vmarkKinectPos[2].x, vmarkKinectPos[3].x));
//	maxPixely = max(max(vmarkKinectPos[0].y, vmarkKinectPos[1].y), max(vmarkKinectPos[2].y, vmarkKinectPos[3].y));



	this->h = h;
	this->h1 = h1;
	this->Radius = Radius;

	this->sCreenPos    =   vmarkscreenpos  ;
	std::vector<V3f> vKinectPos = vmarkKinectPos;

	for (int i = 0; i < vKinectPos.size(); i++) {
		V3f p = vKinectPos[i];
		vKinectPos[i] = getKinectPos(p);
	}

	
	this->vKinectPos = vKinectPos;


	//����Linect����ϵ�µ������ʶ�������������//
	iMaxLimitx = max(min(vKinectPos[0].x, vKinectPos[1].x), min(vKinectPos[2].x, vKinectPos[3].x));
	iMinLimitx = min(max(vKinectPos[0].x, vKinectPos[1].x), max(vKinectPos[2].x, vKinectPos[3].x));

	iMaxLimitz = max(max(vKinectPos[0].z, vKinectPos[1].z), max(vKinectPos[2].z, vKinectPos[3].z));
	iMinLimitz = min(min(vKinectPos[0].z, vKinectPos[1].z), min(vKinectPos[2].z, vKinectPos[3].z));


	V3f v1, v2, v3, v4,v_1, v_2, v_3, v_4;
	v1 = vKinectPos[0];
	v2 = vKinectPos[1];
	v3 = vKinectPos[2];
	v4 = vKinectPos[3];

	std::vector<float> vLength;
	float length1 = (v2 - v1).length();
	float length2 = (v3 - v2).length();
	float length3 = (v4 - v3).length();
	float length4 = (v1 - v4).length();
	float del1 = abs(length1 - 650);
	float del2 = abs(length2 - 920);
	float del3 = abs(length3 - 650);
	float del4 = abs(length4 - 920);   //�����߶�270mm//  //��������Ϊ400mm,���ֱ�Ϊ��14/13/8/5mm//

	Plane3f screenPlane(v1,v2,v3);
	pScreenPlane = screenPlane;


	v_1 = v1;
	v_2 = v2;
	v_3 = v3;
	v_4 = v4;
	Plane3f pDistScreen = GetPlane(v_1, v_2, v_3, v_4);
	
	vdisMarkPoint.push_back(v_1);
	vdisMarkPoint.push_back(v_2);
	vdisMarkPoint.push_back(v_3);
	vdisMarkPoint.push_back(v_4);

	this->iMaxDepth = getMaxDepth();
	this->iMinDepth = getMinDepth();

	this->pdisScreenPlane = pDistScreen;


	//test:������ƽ��ͶӰ�ĵ����Kinectx��ֵ������ͬ//
	V3f vTestPixel(399, 236, 1298);
	V3f KinectPos = getKinectPos(vTestPixel);
	V3f vProjectPoint = getProjectPos(KinectPos);
	V3f vScreenPos = convertTo(vProjectPoint);

}

void CDepthBasics::ProcessDepth(INT64 nTime, const UINT16* pBuffer, int nWidth, int nHeight, USHORT nMinDepth, USHORT nMaxDepth)
{

	int pos = 0;
	if (depthPixelx >= 0 && depthPixely >= 0) {
		pos  = (depthPixely * nWidth + depthPixelx);
	}

	if (m_hWnd)
	{
		if (!m_nStartTime)
		{
			m_nStartTime = nTime;
		}

		double fps = 0.0;

		LARGE_INTEGER qpcNow = { 0 };
		if (m_fFreq)
		{
			if (QueryPerformanceCounter(&qpcNow))
			{
				if (m_nLastCounter)
				{
					m_nFramesSinceUpdate++;
					fps = m_fFreq * m_nFramesSinceUpdate / double(qpcNow.QuadPart - m_nLastCounter);
				}
			}
		}


		WCHAR szStatusMessage[1024];

		//show mouse depth//
		USHORT srcDepth = *(pBuffer + pos);
		//show mouse depth//

/*		StringCchPrintf(szStatusMessage, _countof(szStatusMessage), L" FPS = %0.2f    useTime = %I64d   depthPixelx = %0.2f   depthPixely = %0.2f  depth = %d\n", fps, (nTime - m_nStartTime), depthPixelx, depthPixely, srcDepth);*/  

		StringCchPrintf(szStatusMessage, _countof(szStatusMessage), L" FPS = %0.2f    useTime1 = %d   useTime2 = %d   useTime3 = %d  useTime4 = %d    useTime = %d\n", fps, useTime1, useTime2, useTime3, useTime4, useTime);

		if (SetStatusMessage(szStatusMessage, 1000, false))
		{
			m_nLastCounter = qpcNow.QuadPart;
			m_nFramesSinceUpdate = 0;
		}
	}

	// Make sure we've received valid data

//	std::fstream out5("D:\\ForthDepth.txt", std::ios::app);   //delete for contemporary//
	

	if (m_pDepthRGBX && pBuffer && (nWidth == cDepthWidth) && (nHeight == cDepthHeight))
	{
		RGBQUAD* pRGBX = m_pDepthRGBX;

		// end pixel is start + width*height - 1
		const UINT16* pBufferEnd = pBuffer + (nWidth * nHeight);

		int iCurse = 0;
		

		

		/*
		//delete for contemporary//
		int pos = (identPixely * nWidth + identPixelx);

		int pos1 = (identPixely1 * nWidth + identPixelx1);

		int pos2 = (identPixely2 * nWidth + identPixelx2);

		int pos3 = (identPixely3 * nWidth + identPixelx3);
		*/


		//for test//

		//may not have five points//


		//tets:��ʾ���е�͹��//
		/*
			
		std::vector<int> vPos;
		for (int i = 0; i < identPixel.size(); i++) {
			int pos = (identPixel[i].y * nWidth + identPixel[i].x);
			vPos.push_back(pos);
		}
		*/
		//tets:��ʾ���е�͹��//
		
		//for test//



		/*
		//delete for contemporary//
		if (printflag) {
			if ((identPixelx > 0 || identPixely > 0) && (identPixelx1 > 0 || identPixely1 > 0) && (identPixelx2 > 0 || identPixely2 > 0) && (identPixelx3 > 0 || identPixely3 > 0)) {
				out5 << identPixelx << " " << identPixely << " " << *(pBuffer + pos) << std::endl;
				out5 << identPixelx1 << " " << identPixely1 << " " << *(pBuffer + pos1) << std::endl;
				out5 << identPixelx2 << " " << identPixely2 << " " << *(pBuffer + pos2) << std::endl;
				out5 << identPixelx3 << " " << identPixely3 << " " << *(pBuffer + pos3) << std::endl;

				printflag = false;

				out5 << std::endl;
				out5 << std::endl;
			}
		}
		*/

		

		
		int TestPos = identPixely*nWidth + identPixelx;

		int TestPos1 = -2;
		int TestPos2 = -2;

		if (videntPixelx.size() >= 2) {
			 TestPos1 = videntPixelx.back() + videntPixely.back() * nWidth;
			 TestPos2 = videntPixelx[videntPixelx.size() - 2] + videntPixely[videntPixelx.size() - 2] * nWidth;
		}
		int Count = 0;
		while (pBuffer < pBufferEnd)
		{
			

			USHORT depth = *pBuffer;

			depthData[Count] = depth;
			Count++;

//			depthData.push_back(depth);   //delete for contemporary//


			// To convert to a byte, we're discarding the most-significant
			// rather than least-significant bits.
			// We're preserving detail, although the intensity will "wrap."
			// Values outside the reliable depth range are mapped to 0 (black).

			// Note: Using conditionals in this loop could degrade performance.
			// Consider using a lookup table instead when writing production code.
			BYTE intensity = static_cast<BYTE>((depth >= nMinDepth) && (depth <= nMaxDepth) ? (depth % 256) : 0);

			if (abs(iCurse - TestPos1) < 2) {
				pRGBX->rgbRed = 255;
				pRGBX->rgbGreen = 0;
				pRGBX->rgbBlue = 0;
			}

			else if (abs(iCurse - TestPos2) <2)
			{
				pRGBX->rgbRed = 255;
				pRGBX->rgbGreen = 0;
				pRGBX->rgbBlue = 0;
			}
			
			else {
				pRGBX->rgbRed = intensity;
				pRGBX->rgbGreen = intensity;
				pRGBX->rgbBlue = intensity;
			}
			

			if (iCurse ==pos) {
				pRGBX->rgbRed = 255;
				pRGBX->rgbGreen = 0;
				pRGBX->rgbBlue = 0;
			}
			
				/*
				//test//
				//show four mark point//
				int imaxsize = vPos.size();
				for (int i = 0; i < imaxsize; i++) {
					if (abs(iCurse - vPos[i])<2 ) {
						if (vPos[i] != 0) {
							pRGBX->rgbRed = 255;
							pRGBX->rgbGreen = 0;
							pRGBX->rgbBlue = 0;
						}
					}
				}
				//show four mark point//
				//test//
				*/
			++pRGBX;
			++pBuffer;
			++iCurse;
		}

		depthData[218000] = 1;

		// Draw the data with Direct2D
		m_pDrawDepth->Draw(reinterpret_cast<BYTE*>(m_pDepthRGBX), cDepthWidth * cDepthHeight * sizeof(RGBQUAD));

		if (m_bSaveScreenshot)
		{
			m_bSaveScreenshot = false;

		}
	}
}

/// <summary>
/// Set the status bar message
/// </summary>
/// <param name="szMessage">message to display</param>
/// <param name="showTimeMsec">time in milliseconds to ignore future status messages</param>
/// <param name="bForce">force status update</param>
bool CDepthBasics::SetStatusMessage(_In_z_ WCHAR* szMessage, DWORD nShowTimeMsec, bool bForce)
{
	INT64 now = GetTickCount64();

	if (m_hWnd && (bForce || (m_nNextStatusTime <= now)))
	{
		SetDlgItemText(m_hWnd, IDC_STATUS, szMessage);
		m_nNextStatusTime = now + nShowTimeMsec;

		return true;
	}

	return false;
}

/// <summary>
/// Get the name of the file where screenshot will be stored.
/// </summary>
/// <param name="lpszFilePath">string buffer that will receive screenshot file name.</param>
/// <param name="nFilePathSize">number of characters in lpszFilePath string buffer.</param>
/// <returns>
/// S_OK on success, otherwise failure code.
/// </returns>
HRESULT CDepthBasics::GetScreenshotFileName(_Out_writes_z_(nFilePathSize) LPWSTR lpszFilePath, UINT nFilePathSize)
{
	WCHAR* pszKnownPath = NULL;
	HRESULT hr = SHGetKnownFolderPath(FOLDERID_Pictures, 0, NULL, &pszKnownPath);

	if (SUCCEEDED(hr))
	{
		// Get the time
		WCHAR szTimeString[MAX_PATH];
		GetTimeFormatEx(NULL, 0, NULL, L"hh'-'mm'-'ss", szTimeString, _countof(szTimeString));

		// File name will be KinectScreenshotDepth-HH-MM-SS.bmp
		StringCchPrintfW(lpszFilePath, nFilePathSize, L"%s\\KinectScreenshot-Depth-%s.bmp", pszKnownPath, szTimeString);
	}

	if (pszKnownPath)
	{
		CoTaskMemFree(pszKnownPath);
	}

	return hr;
}

/// <summary>
/// Save passed in image data to disk as a bitmap
/// </summary>
/// <param name="pBitmapBits">image data to save</param>
/// <param name="lWidth">width (in pixels) of input image data</param>
/// <param name="lHeight">height (in pixels) of input image data</param>
/// <param name="wBitsPerPixel">bits per pixel of image data</param>
/// <param name="lpszFilePath">full file path to output bitmap to</param>
/// <returns>indicates success or failure</returns>
HRESULT CDepthBasics::SaveBitmapToFile(BYTE* pBitmapBits, LONG lWidth, LONG lHeight, WORD wBitsPerPixel, LPCWSTR lpszFilePath)
{
	DWORD dwByteCount = lWidth * lHeight * (wBitsPerPixel / 8);

	BITMAPINFOHEADER bmpInfoHeader = { 0 };

	bmpInfoHeader.biSize = sizeof(BITMAPINFOHEADER);  // Size of the header
	bmpInfoHeader.biBitCount = wBitsPerPixel;             // Bit count
	bmpInfoHeader.biCompression = BI_RGB;                    // Standard RGB, no compression
	bmpInfoHeader.biWidth = lWidth;                    // Width in pixels
	bmpInfoHeader.biHeight = -lHeight;                  // Height in pixels, negative indicates it's stored right-side-up
	bmpInfoHeader.biPlanes = 1;                         // Default
	bmpInfoHeader.biSizeImage = dwByteCount;               // Image size in bytes

	BITMAPFILEHEADER bfh = { 0 };

	bfh.bfType = 0x4D42;                                           // 'M''B', indicates bitmap
	bfh.bfOffBits = bmpInfoHeader.biSize + sizeof(BITMAPFILEHEADER);  // Offset to the start of pixel data
	bfh.bfSize = bfh.bfOffBits + bmpInfoHeader.biSizeImage;        // Size of image + headers

	// Create the file on disk to write to
	HANDLE hFile = CreateFileW(lpszFilePath, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	// Return if error opening file
	if (NULL == hFile)
	{
		return E_ACCESSDENIED;
	}

	DWORD dwBytesWritten = 0;

	// Write the bitmap file header
	if (!WriteFile(hFile, &bfh, sizeof(bfh), &dwBytesWritten, NULL))
	{
		CloseHandle(hFile);
		return E_FAIL;
	}

	// Write the bitmap info header
	if (!WriteFile(hFile, &bmpInfoHeader, sizeof(bmpInfoHeader), &dwBytesWritten, NULL))
	{
		CloseHandle(hFile);
		return E_FAIL;
	}

	// Write the RGB Data
	if (!WriteFile(hFile, pBitmapBits, bmpInfoHeader.biSizeImage, &dwBytesWritten, NULL))
	{
		CloseHandle(hFile);
		return E_FAIL;
	}

// Close the file
CloseHandle(hFile);
return S_OK;
}




int CDepthBasics::SeekHands(cv::Mat& cv16img, cv::Mat& cv8img)
{
  

//	return 0;
	//����ǰһ֡��up�������������Ϊ��ǰ֡�Ĳο�//
	std::vector<sAction> vCopyPreHands;
	for (int i = 0; i < m_hands.size(); i++) {
		if (m_hands[i].st != Up) {
			vCopyPreHands.push_back(m_hands[i]);
		}
	}
	m_hands.clear();

	//test//
	//���ö�ֵͼ��ȡ����//
	g_pGrayImage = &IplImage(cv8img);

	//���Ҷ�ͼת���ɶ�ֵͼ//
	g_pBinaryImage = cvCreateImage(cvGetSize(g_pGrayImage), IPL_DEPTH_8U, 1);
	std::vector<vector<Point>> contours;
	on_trackbar(0, contours, cv16img);
	//for test//


	//	std::vector< std::vector<Point> > contours;
	//	cv::findContours(cv8img, contours, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE);  //call findContours to get contours//
#define HAND_LIKELY_AREA 60


	for (unsigned i = 0; i < contours.size(); i++)
	{
		//ֻ���ڼ�⵽����ʱ�Ż�ȥ�����Ķ���Σ�͹���������ݼ�


		cv::Mat contour_mat = cv::Mat(contours[i]);

		//		drawContours(cv8img, contours, i, Scalar(255, 0, 0), 1, 8); //��������

		//test : get contour area//
		float area = contourArea(contour_mat);

		if (contourArea(contour_mat) > HAND_LIKELY_AREA)
		{   //�Ƚ��п������ֵ�����

			//			CalcAveDepth(cv16img, contours[i]);  //��ͼ��ı�Ե��ͨ�����ַ�ʽ�޳�����ȱ任�쳣�ĵ㣬��ȡ��ƽ�����//


			//			cv::Mat cvDrawImg(cDepthHeight, cDepthWidth, CV_8UC1);


			cv::Point3f imgpt;

			cv::Point2f cen; float radius = 0;
			minEnclosingCircle(contours[i], cen, radius);

			//test//
			//����������С���ԲԲ�ĺ������ı߽���ֵ��ȷ�����Ƶı߽��//

			std::vector<Point> MyContours;

			float dist = 0, dist1 = 0;
			cv::Point pMyPoint;

			std::vector<int> oridinal;

			std::vector<int> vdelneighbourPoint;  //�޳������Ѿ����ڵ���MyCotours�е������еĵ�//

			//�����п��ܳ��ֵģ��ҵ��Ľϴ�ֵ�������Ѿ��ҵ��ı߽����������������ʹ�ú���ķ���Ҫ�ı߽��������������Ұ�У���ȡ�ķ���Ϊ���趨�����ֵ���������Ŀ���������ֵ������Ϊ�õ�����СԲ���ڲ��㣬����ȡ��//
			
			while (MyContours.size() < 5 && (oridinal.size()< iPointthreshold)) {
				bool bAddVectorElement = true;
				int imaxindex = 0;
				for (int j = 0; j < contours[i].size(); j++) {
					int k = 0;
					bool flag = true;


					//�޳���ȡ���Ľϴ�dist//
					while (k < oridinal.size()) {
						if (contours[i][oridinal[k]] == contours[i][j]) {
							flag = false;
							break;
						}
						k++;
					}

					if (flag) {
						cv::Point p = contours[i][j];
						dist1 = sqrt((p.x - cen.x)*(p.x - cen.x) + (p.y - cen.y)*(p.y - cen.y));

						if ((dist1 > dist) && (dist1 <= radius)) {
							dist = dist1;
							pMyPoint = p;
							imaxindex = j;
							
						}
					}
				}


				oridinal.push_back(imaxindex);  //////////////////////////Ϊ�����������ڴ�����MyCouters�Լ��޳����ĵ㶼����oridinal��
				dist = 0;
				for (int i = 0; i < MyContours.size(); i++) {
					cv::Point pMyContours = MyContours[i];
					if ((abs(pMyPoint.x - pMyContours.x) <= 10) && (abs(pMyPoint.y - pMyContours.y) <= 10)) {                         
						bAddVectorElement = false;
						break;
					}
				}

				//��һ���ж���䣬��������������ıȽϽ���ֻѡ������һ����Ϊĩ�����ص�//
				if (bAddVectorElement) {
					MyContours.push_back(pMyPoint);
				}
			}
			 
			//test//
			//�Ի�ȡ�����������Ƚ�������//


//			depthRevise(cv16img, contours[i], MyContours);     //delete for contemporary for test the K-means way//


			//�Ի�ȡ�����������Ƚ�������//
			//test//


//			GetHandPos(cv16img, MyContours, imgpt);    //delete for contemporary retrieve the summitPoint of hand//


			//test//
			//���Զ���������ȵ���//

			std::vector<cv::Point> vCopyPreContours;
			vCopyPreContours = MyContours;

			//test1:����ǰ��Ա�//
			std::vector<float>vPreDepth;
			float depth = 0;
			for (int i = 0; i < MyContours.size(); i++) {
				depth = cv16img.at<USHORT>(MyContours[i].y, MyContours[i].x);
				vPreDepth.push_back(depth);
			}
			//test:����ǰ��Ա�//

			depthRevise(cv16img, contours[i], MyContours);


			//test:�������ֵ�Ƿ��и���//
			std::vector<float>vAfterDepth;
	
			for (int i = 0; i < MyContours.size(); i++) {
				depth = cv16img.at<USHORT>(MyContours[i].y, MyContours[i].x);
				vAfterDepth.push_back(depth);
			}
			
			//test:�������ֵ�Ƿ��и���//

			//��˵�仯�ϴ�ĵ���С�ķ����ĵ�һ������ָ��˵�//  //��ƽ��ֵ�͸�����ֵ�����Сʱ��˵��������ֵ�ǿɿ��ģ�����ʹ�ã���ʱ�򿴼��ֵ��ƽ��ֵ�Ĺ�ϵ������Ǳ�С�ģ������С�Ľϴ�һ��Ϊ��˵�//

			//�õ���˵�����һ�ַ���������Ҫ�����Ƽ�����Ե��֮���������Ȳ���Բ��ʺ�����������һ�ߵ����//
			//test:��������ȷ����˵ķ�������Ч��//


			//test//
			//��������������Ļһ�������//
			
			//������վ����Ļ��һ��//   //һ�������������ߣ������������³�ȥ����˲���Ҫ��������y�����жϣ���ö�ͳһΪ����ֵ���ж�//
			//��Ϊ��һ��ʱ�����Ƽ�˵�x���ص�����Զ��ñߣ������������ߣ���ѡ���Ӧ����߽�Զ��x���صĵ㣬������ұߣ���ѡ�����ұ߽�Զ��x���ص��Ӧ�ĵ�// 
			bool bOneSideflag = getSummitPoint(fActorOnOneSidethreshold, MyContours, fSummitPoint,cv16img);  
			                                                                                                    

			bool bInScreenFlag = true;

			

			//*������ȵ����㷨��ȡ�����Ƽ�˵�*//


			if (!bOneSideflag) {
				
				std::vector<float> vDelAfetrToPre;
				std::vector<float>vCopyPreDepth;

				float del = 0;
				for (int i = 0; i < vCopyPreContours.size(); i++) {
					for (int j = 0; j < MyContours.size(); j++) {
						if (vCopyPreContours[i] == MyContours[j]) {
							vCopyPreDepth.push_back(vPreDepth[i]);
							break;
						}
					}
				}

           
				for (int i = 0; i < vAfterDepth.size(); i++) {
					vDelAfetrToPre.push_back(vAfterDepth[i] - vCopyPreDepth[i]);
				}

				float MinIndex = -1;
				float MinDel = 0;
				for (int i = 0; i < vDelAfetrToPre.size(); i++)
				{
                   if (vDelAfetrToPre[i] < MinDel) {
	                     MinDel = vDelAfetrToPre[i];
	                     MinIndex = i;
                    }
				}

				//���õ����Ⱥ�ƽ��ֵ����Ȳ�����ֵ���ڣ�������Ϊ�õ������ǿɿ��ģ����Բ���Ҫ����Ƚ�������//

				//�Ȳ�����ȵļ�С�ı仯���������Ƶ�����¿���//

				/*

				if (MinIndex > 0 && abs(MinDel) < depthrevisethreshold) {
				fSummitPoint = MyContours[MinIndex];
				bInScreenFlag = false;

				//���Բ���һ�£���������������յ�����Ӱ��//
				for (int i = 0; i < MyContours.size(); i++) {
				cv16img.at<USHORT>(MyContours[i].y, MyContours[i].x) = vCopyPreDepth[i];
				}

				}

				else {
				bInScreenFlag = true;
				}

				*/

				//*������ȵ����㷨��ȡ�����Ƽ�˵�*//


				//test//
				//����K-Means������ȡ�����Ƽ�˵�//
				if ((MyContours.size() > 0)) {
					//test//
					//���ڻ�ȡ��ָ���λ��//

					fSummitPoint = getSummitPoint(contours[i], cv16img, MyContours);    //���ڻ�ȡ��˵�����//
					//���ڻ�ȡ��ָ���λ��//
				}
				//����K-Means������ȡ�����Ƽ�˵�//
				//test//
			}

			/*
			//test//
			//����ȷ�����������ɪЧ���Ƿ�ã����Ǳ�ĸ�����//
			//���Խ����������һ���ֵ���ȴ��ڱ仯��Խϴ���������ȱ����//

			//���ڻ�ȡδ����ǰ�ļ�˵�����//
			int PreSummitPointIndex = 0;
			for (int i = 0; i < vCopyPreContours.size(); i++) {
			if (fSummitPoint == vCopyPreContours[i]);
			PreSummitPointIndex = i;
			}
			srcSummitPosDepth = vPreDepth[PreSummitPointIndex];
			//���ڻ�ȡδ����ǰ�ļ�˵�����//



			//���ڻ�ȡδ����ǰ�ļ�˵�����//
			reviseSummitPosDepth = cv16img.at<USHORT>(fSummitPoint.y, fSummitPoint.x);
			//���ڻ�ȡδ����ǰ�ļ�˵�����//

			//����ȷ�����������ɪЧ���Ƿ�ã����Ǳ�ĸ�����//
			//test//
			*/

		

			identPixelx = fSummitPoint.x;
			identPixely = fSummitPoint.y;


			//for test//
			videntPixelx.push_back(fSummitPoint.x);
			videntPixely.push_back(fSummitPoint.y);


			//ps:����״̬��ʧ�ĵ��״̬������ǰ���״̬���//

			//test:��������ȷ����˵ķ�������Ч��//
			//�õ���˵�����һ�ַ���������Ҫ�����Ƽ�����Ե��֮���������Ȳ���Բ��ʺ�����������һ�ߵ����//


			//���Զ���������ȵ���//
			//test//

			//*********************problem***************************//

			//Kinect����������ݻ�û�в⣬���ڵ������ǣ���ĳ������£���⵽���������ڱ仯�ģ���˴���ʶ��ı�Ե��׼ȷ�������Ҫ����һ��//

			//*********************problem***************************//



			//test :����ָ���ض��Ǳ߽��õ�ָ����λ��//

			//test :����ָ���ض��Ǳ߽��õ�ָ����λ��//

			identPixel = MyContours;

			imgpt.x = fSummitPoint.x;
			imgpt.y = fSummitPoint.y;
			imgpt.z = cv16img.at<USHORT>(fSummitPoint.y, fSummitPoint.x);
			



			float PixelDist = 0;
			float fMinPixelDist = 10000;
			int iMinPixelDistIndex = -1;
			V3f p;
			V3f PixelPos;
			for (int i = 0; i < vCopyPreHands.size(); i++) {
				
				p = V3f(vCopyPreHands[i].worldPos.x, vCopyPreHands[i].worldPos.y, vCopyPreHands[i].worldPos.z);
				PixelPos = getPixelPos(p);

				PixelDist = abs(fSummitPoint.x - PixelPos.x) + abs(fSummitPoint.y - PixelPos.y);
				if (PixelDist < fMinPixelDist) {
					fMinPixelDist = PixelDist;
					iMinPixelDistIndex = i;
				}

			}

			if (iMinPixelDistIndex >= 0) {
				p = V3f(vCopyPreHands[iMinPixelDistIndex].worldPos.x, vCopyPreHands[iMinPixelDistIndex].worldPos.y, vCopyPreHands[iMinPixelDistIndex].worldPos.z);
				PixelPos = getPixelPos(p);
				if (abs(fSummitPoint.x - PixelPos.x) < 5 && abs(fSummitPoint.y - PixelPos.y) < 5) {
					if (abs(fSummitPoint.x - PixelPos.x) + abs(fSummitPoint.y - PixelPos.y) < 8) {
						imgpt.z = vCopyPreHands[iMinPixelDistIndex].worldPos.z;
					}
				}
			}





			V3f vImgpt;
			vImgpt.x = imgpt.x;
			vImgpt.y = imgpt.y;
			vImgpt.z = imgpt.z;
	

			

			//test//
			//�����ж������Ƿ���ʶ��������//
			//��Ϊ�������򲢲������ǵ�ʶ����������ڴ˵ػ���Ҫ�жϸ����ĵ��Ƿ���ʶ�����򣬽����жϣ���������������õ���Ա�ʶ�𵽣����Ǹõ㲻��ʶ�������У������Ҫ�Ե��Ƿ���ʶ����������ж�//

			V3f KinectPos = getKinectPos(vImgpt);

			//��ȡ���Ƶ��Kinectpos,Ϊȷ�����Ƶ��ǰ��֡����//
			vfKinectPos.push_back(KinectPos);  //�ж���ֻ�֣�vfKinectPos��size���ж��//

			//�����ж������Ƿ���ʶ��������//
			//test//
		}
	}
	//�����Ƕ������������м��//

	//test//
	//����fSummitPoint������//
	
	std::fstream out2("D:\\fSummitPoint.txt", std::ios::app);
	out2 << fSummitPoint.x << " " << fSummitPoint.y << " "<<cv16img.at<USHORT>(fSummitPoint.y, fSummitPoint.x) << std::endl;
	



	//************************** need to consider further *************************//
	//��ȡǰ��֡�����Ƶ�KinectPos//  //��һ����Ҫ���ǵ���m_hands.size() ��һ����//        //����vPos��ô��ȡ����һ�����ݵģ����vPos��sizeΪ0�����õ�ǰ��m_Hands��ȡ��һ�����ݣ����vPos��size��Ϊ0��������ǰһ֡�͵�ǰ֡������ϻ�ȡvPos������//


	//��Ϊ�ǳ�ʼ֡ʱ//
	if (vPos.size() > 0) {
		float fDist = 0;
		float fMinDist = 1000;
		int iMinIndex = 0;
		cv::Point3f pKinectPos;


		//�����ù̶�����ֵ�����жϣ����������ô洢��hand���speed������ֵ���и���//
		vHandPosThreshold.resize(vCopyPreHands.size());
		for (int i = 0; i < vCopyPreHands.size(); i++) {
			vHandPosThreshold[i] = fHandPosThreshold;
		}
		//�����ù̶�����ֵ�����жϣ����������ô洢��hand���speed������ֵ���и���//


		//��һ֡�͵�ǰ֡��pos����ƥ��ķ����� ��������ǰһ֡�����ƺ͵�ǰ֡�����ƽ��бȽϣ���ȡ����ǰ֡��ǰһ֡���Ƶĵ�Ķ�Ӧ��ϵ���Ӷ�����vidPospush_back(ID/pos),Ȼ����ں�����µ�m_Hands��vpos���裬��ǰһ֡û����������ʱ//
		//�������õ�ǰ��m_Hands�Զ�����//

		//***********************���ͬʱ�������ƶ�ʧ�����µ����ƽ�������ô����**************************//
		//�����������ƶ�ʧ���������뿪��ͬʱ�������ƽ�����������ֵ�����ж��������һ֡�ж������Ƶ�����Ļ����͵�ǰ֡�������������ֵ��2��������Ϊ����ͬһ�����ƣ�������ΪvCopyPreHands[k]Ϊ��ʧ�������ƣ�m_HandsΪ����������//

		//*************�����Ѿ���ȡ��ǰһ֡���Ƶ���ֵ:vHandPosThreshold��������һ��Ҫ�����***************//
		//vHandPosThreshold:��һ��Ĭ��ֵ����������µ�����֮��ľ��룬ͬʱ��Բ�ͬ��hand�˶�������費ͬ��ֵ������ֻ���ǳ����ٶ�������ڳ�ʼ�����趨���������ԸĽ�//


		//1:ǰһ֡�����Ʊȵ�ǰ֡������Ҫ��//
		//����:����ǰһ֡��m_HandsΪ��������ID�ǰ���ǰһ֡˳����������vPos�Ķ�Ӧ��ϵ�в���ҪID//


		//Ϊ�˲�����������ң������Ƚ���ǰ֡��pos����˳�����ͳһ����vPos//

		std::vector<cv::Point3f> vfPos;  //���ڼ�¼��ǰһ֡IDƥ���pos//
		std::vector<int> vMatchIndex;


		if (vCopyPreHands.size() < vfKinectPos.size()) {
			for (int i = 0; i < vCopyPreHands.size(); i++) {
				for (int j = 0; j < vfKinectPos.size(); j++) {

					//�Ѿ�ƥ��õĵ�Ͳ�����ƥ����//
					bool bIsMatch = false;
					for (int k = 0; k < vMatchIndex.size(); k++) {
						if (vMatchIndex[k] == j) {
							bIsMatch = true;
							break;
						}
					}
					//�Ѿ�ƥ��õĵ�Ͳ�����ƥ����//
					if (!bIsMatch) {
						pKinectPos = cv::Point3f(vfKinectPos[j].x, vfKinectPos[j].y, vfKinectPos[j].z);
						cv::Point3f pPreKinectPos = cv::Point3f(vCopyPreHands[i].worldPos.x, vCopyPreHands[i].worldPos.y, vCopyPreHands[i].worldPos.z);
						cv::Point3f distp = pKinectPos - pPreKinectPos;
						fDist = sqrt(distp.x * distp.x + distp.y * distp.y + distp.z * distp.z);
						if (fMinDist > fDist) {
							fMinDist = fDist;
							iMinIndex = j; 
						}
					}

				}

				//���ڴ�����ֵ��2�����Σ�ǰһ֡Ϊ��ʧ���߳�ȥ��handPos����ǰ֡��Ӧ��handΪ������handPos,������Ҫ�Ͷ�Ӧ��Hand�ľ�����ֵ����ƥ��//
				if (fMinDist > 2 * vHandPosThreshold[i]) {
					vfPos.push_back(cv::Point3f(i, -1, -1));  //��ʾ��Ӧ��ǰһ֡Ϊ��ʧ�Ĺؼ�֡��-1��-1Ϊ��־λ//
				}

				else {
					vMatchIndex.push_back(iMinIndex);
					vfPos.push_back(cv::Point3f(vfKinectPos[iMinIndex].x, vfKinectPos[iMinIndex].y, vfKinectPos[iMinIndex].z));
				}

				fMinDist = 10000;
				iMinIndex = 0;
			}
			//����j��δ�����posֱ�Ӽӵ�vfPos�ĺ���//
			
			//���õ�˼��Ϊ����vMathcIndex��û�е�Index�����¶���������ƣ���Ҫ�����·����//
			for (int k = 0; k < vfKinectPos.size(); k++) {
				int l = 0;
				bool flag = true;
				while (l < vMatchIndex.size() && flag) {
					if (k == vMatchIndex[l]) {
						flag = false;
					}
					l++;
				}
				if (flag) {
					vfPos.push_back(cv::Point3f(vfKinectPos[k].x, vfKinectPos[k].y, vfKinectPos[k].z));  
				}
			}
			vMatchIndex.clear();   //�κ�һ��vector����֮��һ��Ҫclear����ֹ�������ĳ������Ӱ��//
		}


		//2:ǰһ֡�����Ʊȵ�ǰ֡�����ƶ�//

		else {

			//��ʼ��vPosӦ�ú�ǰһ֡��m_Hand������ͬ��size��Ȼ�������NewHands��vfPos��������//
			vfPos.resize(vCopyPreHands.size());
			//vfPos�ĳ�ʼ������ǰһ֡�����Ƶ��ڵ�ǰ֡���ǲ����ڵ�//
			for (int i = 0; i < vfPos.size(); i++) {
				vfPos[i] = cv::Point3f(i, -1, -1);          
			}


			//����1��ƥ�䣬�ҵ���Ӧ��ϵ//

			//���ڵ�ǰ֡ƥ���ϵ�����ǰһ֡�ľ��������ֵ������pos����¼���������µ�ID//
			std::vector<cv::Point3f> vCurrentNewHand;

			for (int i = 0; i < vfKinectPos.size(); i++) {
				for (int j = 0; j < vCopyPreHands.size(); j++) {

					//�Ѿ�ƥ��õĵ�Ͳ�����ƥ����//
					bool bIsMatch = false;
					for (int k = 0; k < vMatchIndex.size(); k++) {
						if (vMatchIndex[k] == j) {
							bIsMatch = true;
							break;
						}
					}
					//�Ѿ�ƥ��õĵ�Ͳ�����ƥ����//

					if (!bIsMatch) {
						pKinectPos = cv::Point3f(vfKinectPos[i].x, vfKinectPos[i].y, vfKinectPos[i].z);
						cv::Point3f pPreKinectPos = cv::Point3f(vCopyPreHands[j].worldPos.x, vCopyPreHands[j].worldPos.y, vCopyPreHands[j].worldPos.z);
						cv::Point3f distp = pKinectPos - pPreKinectPos;
						fDist = sqrt(distp.x * distp.x + distp.y * distp.y + distp.z * distp.z);
						if (fMinDist > fDist) {
							fMinDist = fDist;
							iMinIndex = j;
						}
					}
				}


				//���ڴ�����ֵ��2�����Σ�ǰһ֡Ϊ��ʧ���߳�ȥ��handPos����ǰ֡��Ӧ��handΪ������handPos,������Ҫ�Ͷ�Ӧ��Hand�ľ�����ֵ����ƥ��//
				if (fMinDist > 2 * vHandPosThreshold[iMinIndex]) {
					vCurrentNewHand.push_back(pKinectPos);                      //��¼�´�ʱ��m_Hands��pos��Ϊ�µ�����//
				}


				else {
					vfPos[iMinIndex] = cv::Point3f(vfKinectPos[i].x, vfKinectPos[i].y, vfKinectPos[i].z);
				}

				fMinDist = 10000;
				iMinIndex = 0;
			}


			//����ǰ֡��ǰһ֡ƥ�䵫�Ǵ�����ֵ��hand��pos����vfPos//
			for (int m = 0; m < vCurrentNewHand.size(); m++) {
				vfPos.push_back(vCurrentNewHand[m]);
			}
			//����ǰ֡��ǰһ֡ƥ�䵫�Ǵ�����ֵ��hand��pos����vfPos//

		}

		//���õõ���vfPos��vPos���и���//
		for (int i = 0; i < vfPos.size(); i++) {
			if (i < vPos.size()) {
				if (vPos[i].size() < 3) {
					vPos[i].push_back(vfPos[i]);
				}

				else {
					vPos[i][vPos[i].size() - 3] = vPos[i][vPos[i].size() - 2];
					vPos[i][vPos[i].size() - 2] = vPos[i][vPos[i].size() - 1];
					vPos[i][vPos[i].size() - 1] = vfPos[i];
				}
			}

			else {
				std::vector<cv::Point3f> VNewHands;
				VNewHands.push_back(vfPos[i]);
				vPos.push_back(VNewHands);
			}

		}

		//���õõ���vfPos��vPos���и���//
	}


	//  //��Ϊ��ʼ֡ʱ//                   //�Ժ�д�����ʱ�����if������Ҫ��elseһ��Ҫ��д�ϣ���ֹ�ں�������////  
	//��ʼ֡��Ӧ��vPos�㼴Ϊ��ǰ��hands��pos//
	else {
		vPos.resize(vfKinectPos.size());
		for (int i = 0; i < vPos.size(); i++) {
			vPos[i].push_back(cv::Point3f(vfKinectPos[i].x, vfKinectPos[i].y, vfKinectPos[i].z));
		}
	}

	//vPos�����ã��Ե�ǰ֡��vPos�У������Ӧ��pos��y,zΪ-1��ʾ�ڵ�ǰ֡��û�и����Ƶģ���Ҫ�Ը������Ƕ�ʧ�����뿪�����жϣ�������������һ֡�Ĵ洢�����Ƶı߽���ֵ//
	//���������ֵ�ķ�Χ֮�⣬�Ըõ�����޸����������ֵ�ķ�Χ֮�ڣ����������ж�Ϊup//

	//�ڴ˶�m_hands���е���//
	//1��������ڱ߽��ϣ������ж�Ϊup����Ϊup//
	//2��:�����in,����Ҫ����//

	float fSpeedLength = 0;
	cv::Point3f pSpeed;
	for (int i = 0; i < vPos.size(); i++) {
		if (vPos[i][vPos[i].size() - 1].y == -1 && vPos[i][vPos[i].size() - 1].z == -1) {
			bool bIsInZone = true;
			if (vPos[i].size() < 2) {

			}

			else {
				std::vector<cv::Point3f> vLastTwoPos;
				vLastTwoPos.resize(2);

				if (vPos[i].size() < 3) {
					vLastTwoPos[0] = cv::Point3f(vPos[i][0].x, vPos[i][0].y, vPos[i][0].z);
					vLastTwoPos[1] = cv::Point3f(vPos[i][0].x, vPos[i][0].y, vPos[i][0].z);
				}

				else   {
					vLastTwoPos[0] = cv::Point3f(vPos[i][0].x, vPos[i][0].y, vPos[i][0].z);
					vLastTwoPos[1] = cv::Point3f(vPos[i][1].x, vPos[i][1].y, vPos[i][1].z);
				}
				pSpeed = vLastTwoPos[1] - vLastTwoPos[0];
				fSpeedLength = sqrt(pSpeed.x * pSpeed.x + pSpeed.y * pSpeed.y + pSpeed.z * pSpeed.z);
				V3f vfPospre, vfPosAfter;
				vfPospre.x = vLastTwoPos[0].x; vfPospre.y = vLastTwoPos[0].y;  vfPospre.z = vLastTwoPos[0].z;
				vfPosAfter.x = vLastTwoPos[1].x; vfPosAfter.y = vLastTwoPos[1].y;  vfPosAfter.z = vLastTwoPos[1].z;

				V3f p = V3f(vPos[i][1].x, vPos[i][1].y, vPos[i][1].z);
				bIsInZone = IsInOrOutOfZone(p, fSpeedLength, vfPospre, vfPosAfter);
			} 

			//true:��ʾ�ڱ�Ե,�ҷ����ǳ��ų�ȥ�ķ���,false��ʾ��ʶ�������м�//
			if (bIsInZone) {
				//��Ӧ�õ�Ϊup��Ҳ��vPos[i][vPos[i].size()-1]���������������־Ϊ����m_hands״̬����Ϊup//
			}
			else {

				//��һ֡������Ҳʶ�𲻵�������õ�û��ǰһ֡�����ݣ���ô�õ�͸��������ڣ�����ǰһ֡�����ݿ϶����е�//
				vPos[i][vPos[i].size() - 1] = vPos[i][vPos[i].size() - 2] + pSpeed;
			}

		}
	}

	//�Ե�ǰ��m_hand���и��� ������vPos//
	m_hands.resize(vPos.size());
	for (int i = 0; i < vPos.size(); i++) {
		m_hands[i].ID = i;
		if (vPos[i][vPos[i].size() - 1].y == -1 && vPos[i][vPos[i].size() - 1].z == -1) {
			//�����Ӧ������Ϊup,���ñ�־Ϊȷ����Ϊup��Ϊ��һ��ɾ������//
			m_hands[i].st = Up;
			m_hands[i].worldPos.x = vPos[i][vPos[i].size() - 2].x;
			m_hands[i].worldPos.y = vPos[i][vPos[i].size() - 2].y;
			m_hands[i].worldPos.z = vPos[i][vPos[i].size() - 2].z;

			//��ȡ��Ӧ����ĻͶӰ��//
			V3f p = V3f(m_hands[i].worldPos.x, m_hands[i].worldPos.y, m_hands[i].worldPos.z);
			V3f pScreenProjectp = getProjectPos(p);
			V3f vD2ScreenProjectp = convertTo(pScreenProjectp);  
			m_hands[i].pos.x = vD2ScreenProjectp.x;
			m_hands[i].pos.y = vD2ScreenProjectp.y;
			m_hands[i].pos.z = vD2ScreenProjectp.z;

		}

		else  {
			V3f p = V3f(vPos[i][vPos[i].size() - 1].x, vPos[i][vPos[i].size() - 1].y, vPos[i][vPos[i].size() - 1].z);

			
			


			bool bIsInFlag = IsInZone(p);  //�����жϵ��Ƿ���ʶ�������ڣ��Ӷ��õ��õ��״̬//
			sAction act;
			act.ID = i;
			if (bIsInFlag) {
				ConvertImagePointToHand(p, act, vCopyPreHands);  //get kinect coordnitate pos//
				
			}

			else
			{
				act.st = Up;
				V3f pScreenProjectp = getProjectPos(p);
				V3f vD2ScreenProjectp = convertTo(pScreenProjectp);
				act.pos.x = vD2ScreenProjectp.x;
				act.pos.y = vD2ScreenProjectp.y;
				act.pos.z = vD2ScreenProjectp.z;
			}

			m_hands[i] = act;
		}
	}
	//��ȡ����ǰ֡������//

	//��һ�����ƿ��ܴ��ڵ�����ǣ���ǰ֡����Ϊup,���ǵ�ǰ֡û�����ƣ���ô���������ǻ���Ϊһ���µ����Ƴ��֣�����ڼ�������Ϊup����Ҫ��vpos������Ӧ�ĸ��£�ɾ����up��Ӧ��pos//
	std::vector<std::vector<cv::Point3f>>   vCopyvPos;
	for (int i = 0; i < m_hands.size(); i++) {
		if (m_hands[i].st != Up) {
			vCopyvPos.push_back(vPos[i]);
		}
	}
	vPos = vCopyvPos;



	//��vPos���и���//
	std::vector<std::vector<cv::Point3f>>  vCopyPos;
	for (int i = 0; i < vPos.size(); i++) {
		if (vPos[i][vPos[i].size() - 1].y != -1 || vPos[i][vPos[i].size() - 1].z != -1) {
			vCopyPos.push_back(vPos[i]);
		}
	}
	vPos = vCopyPos;

	vfKinectPos.clear();

	if (m_hands.size() == 0) {
		return (int)contours.size();
	}

	//test//
	//���ڶ������Ƶ�ȷ��//
	if (m_hands.size() >0) {
		if (vCopyPreHands.size() > 0)  {
			if (fabs(m_hands[0].worldPos.z - vCopyPreHands[0].worldPos.z) >20) {

				cv::Point3f p1 = cv::Point3f(m_hands[0].worldPos.x, m_hands[0].worldPos.y, m_hands[0].worldPos.z);
				cv::Point3f pPixelPos = cv::Point3f(getPixelPos(V3f(p1.x, p1.y, p1.z)).x, getPixelPos(V3f(p1.x, p1.y, p1.z)).y, getPixelPos(V3f(p1.x, p1.y, p1.z)).z) ;

				cv::Point3f p2 = cv::Point3f(vCopyPreHands[0].worldPos.x, vCopyPreHands[0].worldPos.y, vCopyPreHands[0].worldPos.z);
				cv::Point3f pPixelPos1 = cv::Point3f(getPixelPos(V3f(p2.x, p2.y, p2.z)).x, getPixelPos(V3f(p2.x, p2.y, p2.z)).y, getPixelPos(V3f(p2.x, p2.y, p2.z)).z);
		}
		
	}
		std::fstream out1("D:\\MoreHands.txt", std::ios::app);
		for (int i = 0; i < m_hands.size(); i++) {
			out1 << m_hands[i].ID << " " << m_hands[i].pos.x << " " << m_hands[i].pos.y << " " << m_hands[i].pos.z << " " << m_hands[i].st << " " << fSummitPoint.x << " " << fSummitPoint.y << " " << m_hands[i].worldPos.z << " " <<  m_hands[i].critpos.x <<" " << m_hands[i].critpos.y <<  std::endl;
		}

		out1 << std::endl;
	}
	//���ڶ������Ƶ�ȷ��//
	//test//


	/*
	//�Ե������ƵĲ���//
	//�������Ƶ�׼ȷ��//
	else {
		std::fstream out("D:\\HandGestrue.txt", std::ios::app);
		for (int i = 0; i < m_hands.size(); i++) {
			out << m_hands[i].pos.x << " " << m_hands[i].pos.y << " " << m_hands[i].pos.z << " " << m_hands[i].st << std::endl;
			if (vPos.size() >0) {
				out << vPos[0][0].x << " " << vPos[0][0].y <<" " << vPos[0][0].z << std::endl;
			}
		}
		out << std::endl;
	}

	//�������Ƶ�׼ȷ��//
	*/
}



//��Ҫ֪��ǰһ֡���������жϵ�ǰ֡������״̬,ǰ���Ǹ����Ʊ�����ʶ��������//
//����up�㶼ɾ���ˣ���Ϊ�Ѿ���ȥ��//
//�����ҵ���ǰ������ǰһ֡��ƥ�����ƣ�//
//1�����ǰһ֡��ƥ�����ƣ��Ǹ�����Ϊdown��//
//2:���ǰһ֡����Ϊdown,���þ�������жϣ�������������ֵ,���ж�ΪMove����Ϊdown;  //�þ��������Ϊ��Ļ����//  //
//3:���ǰһ֡״̬ΪMove,��ǰ֡������ǰһ֡���������ֵ������ΪMove������Ϊnone//
//4: ���ǰһ֡������Ϊnone��˵��ǰһ֡���ڵȴ�״̬�У��������Ļ�ϵ��ƶ��������Radius,����Ϊ���Move,������Ϊnone//

//5:��������Move��ʱ��������ȴ������Լ�������������Ҳ����������趨���˶�����ֵΪ25mm������Ŀ��Ը��ݾ����������иı䣬��ǰһ֡ΪNone��ʱ���жϵľ���ı�׼Ϊ�����һ��Move�����ݣ�Ϊdownʱ���ж�Move������Ҳ�����һ֡Ϊdown������//
void CDepthBasics::ConvertImagePointToHand(const V3f& p, sAction& act,const std::vector<sAction> preHands)
{
	int index = 0;
	V3f pScreenProjectp = getProjectPos(p);
	V3f pMid = convertTo(pScreenProjectp);

	cv::Point3f pKinectPos = cv::Point3f(p.x, p.y, p.z);
	bool  bHasMatchPoint = getMarchLastHandPos(preHands, pKinectPos, fHandPosThreshold, index);
	if (!bHasMatchPoint) {
		act.st = Down;
		act.critpos.x = pMid.x;
		act.critpos.y = pMid.y;
	}

	else {

		if (preHands[index].st ==  Move)
		{
			act.critpos = preHands[index].pos;
		}
		else {
			act.critpos = preHands[index].critpos;
		}


		float distance = sqrt((pMid.x - act.critpos.x) * (pMid.x - act.critpos.x) + (pMid.y - act.critpos.y) * (pMid.y - act.critpos.y));
		if (preHands[index].st == Down) {
			if (distance > Radius) {
				act.st = Move;
			}
			else {
				act.st = Down;
			}

		}

		else if (preHands[index].st == Move) {
			if (distance > Radius) {
				act.st = Move;
			}
			else {
				act.st = None;
			}
		}

		else if (preHands[index].st == None){
			if (distance > Radius) {
				act.st = Move;
			}
			else {
				act.st = None;
			}
		}

		if (preHands.size() == 0) {
			if (act.st == Down && preHands[0].st == None) {
			}

		}

	}

	

	//��ȡ��ǰ���Ƶ���Ļ����//
	//�����ǰ֡��Ӧ������ΪDown��ǰһ֡��Ӧ������ҲΪDown���ǵ�ǰ֡��ScreenPos��ǰһ֡�����worldPosҲ��ǰһ֡���//
	/*
	if (bHasMatchPoint) {
		if ((act.st == Down && preHands[index].st == Down) || (act.st == None  && preHands[index].st == Move) || (act.st == None  && preHands[index].st == None))
		{
			act.pos.x = preHands[index].pos.x;
			act.pos.y = preHands[index].pos.y;
			act.pos.z = preHands[index].pos.z;

			//��ȡ��ǰ���Ƶ�ȫ������//
			act.worldPos.x = preHands[index].worldPos.x;
			act.worldPos.y = preHands[index].worldPos.y;
			act.worldPos.z = preHands[index].worldPos.z;
		}


		else {
			act.pos.x = pMid.x;
			act.pos.y = pMid.y;
			act.pos.z = pMid.z;

			//��ȡ��ǰ���Ƶ�ȫ������//
			act.worldPos.x = p.x;
			act.worldPos.y = p.y;
			act.worldPos.z = p.z;
		}
	}

	*/

//	else {
		act.pos.x = pMid.x;
		act.pos.y = pMid.y;
		act.pos.z = pMid.z;

		//��ȡ��ǰ���Ƶ�ȫ������//
		act.worldPos.x = p.x;
		act.worldPos.y = p.y;
		act.worldPos.z = p.z;
//	}


		//�������ֵ�ı仯����Ƿ��ȶ�//
// 		std::fstream out2("D:\\DepthChangeTest.txt", std::ios::app);
// 		out2 << p.x << " " << p.y <<" "<< p.z << std::endl;

}



V3f CDepthBasics::getKinectPos(const V3f & pos) 
{

	V3f p;
	p.x = (pos.x - 256) *pos.z / foc;
	p.y = (pos.y - 212)  *pos.z / foc;
	p.z = pos.z;
	return p;
}

//����Kinect�����ȡ���ص�����//
V3f CDepthBasics::getPixelPos(const V3f & pos) {
	V3f p;
	p.x = pos.x * foc / pos.z + 256;
	p.y = pos.y * foc / pos.z + 212;
	return p;
}

V3f CDepthBasics::getRatio(const V3f &a, const V3f &b, const V3f & c)
{
	V3f point;

	float k11 = a.x * a.x + a.y * a.y + a.z * a.z;
	float k12 = a.x * b.x + a.y * b.y + a.z * b.z;
	float k13 = a.x * c.x + a.y * c.y + a.z *c.z;

	float k21 = a.x * b.x + a.y * b.y + a.z * b.z;
	float k22 = b.x * b.x + b.y * b.y + b.z * b.z;
	float k23 = b.x * c.x + b.y * c.y + b.z *c.z;

	point.x = (k13*k22 - k23*k12) / (k11*k22 - k21*k12);
	point.y = (k11*k23 - k21*k13) / (k11*k22 - k21*k12);
	return point;
}

//����Ļ��kinect��ά����ת������Ļ�Ķ�ά����//
V3f   CDepthBasics::convertTo( const V3f& pos)
{
	V3f point1, point2, point3, a, b, c;
	point1 = vKinectPos[0];
	point2 = vKinectPos[1];
	point3 = vKinectPos[3];

	a = point2 - point1;
	b = point3 - point1;
	c = pos - point1;

	V3f K = getRatio(a, b, c);
	V3f point_1, point_2, point_3, a1, a2, a3;
	point_1 = sCreenPos[0];
	point_2 = sCreenPos[1];
	point_3 = sCreenPos[3];
	a1 = point_2 - point_1;
	a2 = point_3 - point_1;
	a3 = K.x * a1 + K.y * a2 + point_1;
	return a3;
}



bool CDepthBasics::Precalc16_fast( cv::Mat & cv16img, cv::Mat& cv8img)
{
// 
// 	for (int i = 0; i <  cDepthHeight; i++) {
// 		for (int j = 0; j < cDepthWidth; j++) {
// 
// 			//��ʵ��ʹ���У���Kinect������Ļ�Ϸ�ʱ��ȷ������ķ�Χ����Kinect��yֵ��ȷ���ģ�������޶���Χ��ʱ�򣬲�����depth���޶�//
// 			//������������ֵ��ͼ��ʶ������߽�����޶�������Ļ��Kinect�Ͻ�ʱ����ʶ���y�������С��������΢����Զһ�㣬�ͻ������Ļʶ������//
// 
// 			if (i< maxPixely && i > minPixely && j < maxPixelx && j > minPixelx && cv16img.at<USHORT>(i, j) < iMaxDepth &&  cv16img.at<USHORT>(i, j) > iMinDepth)
//  			{
// 					V3f pImgKinectPos = getKinectPos(V3f(j, i, cv16img.at<USHORT>(i, j)));
// 
// 
// 					V3f projectP = getProjectPos(pImgKinectPos);
// 
// 					float fdist = (projectP - pImgKinectPos).length();
// 
// 					if (fdist > 50 && fdist < h){
// 						cv8img.at<uchar>(i, j) = (uchar)(cv16img.at<USHORT>(i, j) / (iMaxDepth + 100)) > 1 ? 0 : (uchar)(cv16img.at<USHORT>(i, j) * 255.0 / (iMaxDepth + 100));
// 					}
// 			}
// 
// 			//������ά��ȥ�ж�������Ƿ���ʶ��������//
// 		}
// 	}
// 
// 	return true;

	
/*	cv::Mat cvimg = cv16img;*/
	for (int i = 0; i < cDepthHeight; i++) {
		for (int j = 0; j < cDepthWidth; j++) {
			V3f pImg;
			pImg.x = j;
			pImg.y = i;
			pImg.z = cv16img.at<USHORT>(i, j);

			//��ʵ��ʹ���У���Kinect������Ļ�Ϸ�ʱ��ȷ������ķ�Χ����Kinect��yֵ��ȷ���ģ�������޶���Χ��ʱ�򣬲�����depth���޶�//
			//������������ֵ��ͼ��ʶ������߽�����޶�������Ļ��Kinect�Ͻ�ʱ����ʶ���y�������С��������΢����Զһ�㣬�ͻ������Ļʶ������//
			V3f pImgKinectPos = getKinectPos(pImg);



			if (pImgKinectPos.x < iMaxLimitx && pImgKinectPos.x > iMinLimitx  && pImgKinectPos.z > iMinLimitz && pImgKinectPos.z < iMaxLimitz)

				//			if ((pImg.x > minPixelx) && (pImg.x < maxPixelx) && (pImg.y > minPixely) && (pImg.y < maxPixely))
				//			if (((pImg.z < iMaxDepth) && (pImg.z > iMinDepth)) && (pImg.x > minPixelx)  && (pImg.x < maxPixelx)  && (pImg.y > minPixely)  && (pImg.y < maxPixely))
			{
				V3f p = getKinectPos(pImg);

				V3f projectP = getProjectPos(p);

				float fdist = (projectP - p).length();


				if (fdist <50 || fdist >h)  {
					cv16Img.ptr<USHORT>(i)[j] = 0;
				}
			}

			else
			{
				cv16Img.ptr<USHORT>(i)[j] = 0;
			}

			//������ά��ȥ�ж�������Ƿ���ʶ��������//
		}
	}
	for (int i = 0; i < cDepthHeight; i++) {
		for (int j = 0; j < cDepthWidth; j++) {
			cv8Img.ptr<uchar>(i)[j] = (uchar)(cv16img.at<USHORT>(i, j) * 255.0 / iMaxLimitz);
		}
	}
	return  true;
	
}




bool CDepthBasics::IsBetweenTwoPlane(const V3f  & v1, const V3f  &v2, const V3f & v3)
{
	float length1, length2, length3;
	length1 = (v2 - v1).length();
	length2 = (v3 - v2).length();
	length3 = (v3 - v1).length();
	if (length1 < length3 && length2 < length3)
		return true;
	return false;
}


//Ϊ����������������ʶ�������Ҳ��ȷ�������С���ֵ���Ӷ�ȷ��ʶ������//
Plane3f CDepthBasics::GetPlane(V3f  & v1, V3f & v2, V3f & v3, V3f & v4)
{
	V3f vScreen1, vScreen2, vScreen3, vScreen4;
	vScreen1 = v1;
	vScreen2 = v2;
	vScreen3 = v3;
	vScreen4 = v4;
	Plane3f screenPlane(v1, v2, v3);

	V3f normalVector = screenPlane.normal;
	v1 = v1 - h * normalVector;
	v2 = v2 - h * normalVector;
	v3 = v3 - h * normalVector;
	v4 = v4 - h * normalVector;
	Plane3f screenPlane1(v1, v2, v3);

	/*
	Line3f line1(V3f(0, 0, 0), vScreen1);
	Line3f line2(V3f(0, 0, 0), vScreen2);
	Line3f line3(V3f(0, 0, 0), vScreen3);
	Line3f line4(V3f(0, 0, 0), vScreen4);
	screenPlane1.intersect(line1, v1);
	screenPlane1.intersect(line2, v2);
	screenPlane1.intersect(line3, v3);
	screenPlane1.intersect(line4, v4);
	*/

	return screenPlane1;
}



bool CDepthBasics::IsInZone(const V3f  & pKinectPos)
{

	//��Ȼ�ǽ�������ʶ�����򣬻�����ͶӰ��ƽ���ϵ�����Ϻ�//

	V3f vfProjectScreen1, vfProjectScreen2;

	vfProjectScreen1 = getProjectPos(pKinectPos);
	vfProjectScreen2 = getProjectPos(pKinectPos, pdisScreenPlane);
	

	

	if (IsBetweenTwoPlane(vfProjectScreen1, pKinectPos, vfProjectScreen2))
	{
		if (GetPositionLocation(vKinectPos[0], vKinectPos[1], vKinectPos[2], vKinectPos[3], vfProjectScreen1))
		{
			if (GetPositionLocation(vdisMarkPoint[0], vdisMarkPoint[1], vdisMarkPoint[2], vdisMarkPoint[3], vfProjectScreen1))
				return true;
		}
	}
	return false;
}



//��ȡ��������������Ļƽ���ͶӰ�㣬��ͶӰ������ΪKinect����ϵ�µ�����//
V3f CDepthBasics::getProjectPos(const V3f & sKinectPos)
{

	//test//
	if (sKinectPos.z != 0) {
		Line3f posLine;
		posLine.pos = sKinectPos;
		posLine.dir = pScreenPlane.normal;
		V3f vProjectPoint;
		pScreenPlane.intersect(posLine, vProjectPoint);
		return vProjectPoint;
	}
}


V3f CDepthBasics::getProjectPos(const V3f & sKinectPos,const Plane3f & plane)
{

	//test//
	if (sKinectPos.z != 0) {
		Line3f posLine;
		posLine.pos = sKinectPos;
		posLine.dir = plane.normal;
		V3f vProjectPoint;
		plane.intersect(posLine, vProjectPoint);
		return vProjectPoint;
	}
}


bool CDepthBasics::GetPositionLocation(const V3f & v1, const V3f & v2, const V3f & v3, const V3f & v4, const V3f & v5)
{	
	float angle;
	V3f vVector1 = v2 - v1;
	V3f vVector2 = v3 - v2;
	V3f vVector3 = v4 - v3;
	V3f vVector4 = v1 - v4;
	std::vector<V3f> vEdgeVector;
	vEdgeVector.push_back(vVector1);
	vEdgeVector.push_back(vVector2);
	vEdgeVector.push_back(vVector3);
	vEdgeVector.push_back(vVector4);

	V3f vVector_1 = v5 - v1;
	V3f vVector_2 = v5 - v2;
	V3f vVector_3 = v5 - v3;
	V3f vVector_4 = v5 - v4;
	std::vector<V3f> vPointEdgeVector;
	vPointEdgeVector.push_back(vVector_1);
	vPointEdgeVector.push_back(vVector_2);
	vPointEdgeVector.push_back(vVector_3);
	vPointEdgeVector.push_back(vVector_4);

	std::vector<float>Angle;

	angle = acos(((v4 - v1).normalize()).dot((v2 - v1).normalize()));
	Angle.push_back(angle);
	angle = acos(((v3 - v2).normalize()).dot((v1 - v2).normalize()));
	Angle.push_back(angle);
	angle = acos(((v4 - v3).normalize()).dot((v2 - v3).normalize()));
	Angle.push_back(angle);
	angle = acos(((v1 - v4).normalize()).dot((v3 - v4).normalize()));
	Angle.push_back(angle);
	for (int i = 0; i < 4; i++) {
		angle = acos((vEdgeVector[i].normalize()).dot((vPointEdgeVector[i]).normalize()));
		if (angle > Angle[i])
			return false;
	}
	return true;
}



bool CDepthBasics::GetHandPos(const Mat & cv16Img, const std::vector<Point>& contour, cv::Point3f  & hand) {

	std::vector<float> vdistance;
	float fInitdis = 100000;
	int  k = 0;

	

	for (int i = 0; i < contour.size(); i++) {
		cv::Point2i pixelPoint = contour[i];
		USHORT depth = cv16Img.at<USHORT>(pixelPoint.y, pixelPoint.x);



		V3f pKinectCoordinatePos(pixelPoint.x, pixelPoint.y, depth);
		pKinectCoordinatePos = getKinectPos(pKinectCoordinatePos);
		V3f pScreenProjectPos(pKinectCoordinatePos);
		   pScreenProjectPos = getProjectPos(pScreenProjectPos);
		V3f del = pScreenProjectPos - pKinectCoordinatePos; 
		float fdis = del.length();


		if (fdis < fInitdis)  {
			k = i;
			fInitdis = fdis;
		}

	}
	hand.x = contour[k].x;
	hand.y = contour[k].y;
	hand.z = cv16Img.at<USHORT>(contour[k].y, contour[k].x );
	return true;
}


void CDepthBasics::CalcAve(const cv::Mat& cv16img, std::vector<Point>& contour, int  & maxdepth, int & mindepth, float  & avg) {
	int num = (int)contour.size();
	int acc = 0;
	maxdepth = 0;
	mindepth = 10000;


	for (int i = 0; i < num; i++)
	{

		int depth = (int)cv16img.at<ushort>(contour[i].y, contour[i].x);
		if (depth > maxdepth) {
			maxdepth = depth;
		}

		if (depth < mindepth) {
			mindepth = depth;
		}


		acc += (int)cv16img.at<ushort>(contour[i].y, contour[i].x);

		float num1 = num;

		avg = acc / num1;
	}
}



float CDepthBasics::CalcAveDepth(const cv::Mat& cv16img, std::vector<Point>& contour)
{

	int   maxdepth = 0, mindepth = 10000;
	float   avg;

	CalcAve(cv16img, contour, maxdepth, mindepth, avg);

	while (abs(maxdepth - avg) > 40 || abs(mindepth - avg) > 40) {

		std::vector<Point> checkContour;
		checkContour.clear();

		if (abs(maxdepth - avg) > abs(mindepth - avg)) {
			if (abs(maxdepth - avg) > 40) {
				int markdepth = maxdepth - 20;
				for (int i = 0; i < contour.size(); i++) {
					if (cv16img.at<ushort>(contour[i].y, contour[i].x) < markdepth) {
						checkContour.push_back(cv::Point(contour[i].x, contour[i].y));
					}
				}
			}
		}

		else {
			if (abs(mindepth - avg) > 40) {
				int markdepth = mindepth + 20;
				for (int i = 0; i < contour.size(); i++) {
					if (cv16img.at<ushort>(contour[i].y, contour[i].x) > markdepth) {
						checkContour.push_back(cv::Point(contour[i].x, contour[i].y));
					}
				}
			}
		}
		contour = checkContour;
		CalcAve(cv16img, contour, maxdepth, mindepth, avg);
	}
	return avg;
}

UINT  CDepthBasics::getMaxDepth() {
	UINT maxDepth = max(max(vKinectPos[0].z, vKinectPos[1].z), max( vKinectPos[2].z,vKinectPos[3].z));
	return maxDepth;
}

UINT  CDepthBasics::getMinDepth() {
	UINT minDepth = min(min(vdisMarkPoint[0].z, vdisMarkPoint[1].z), min(vdisMarkPoint[2].z, vdisMarkPoint[3].z));
	return minDepth;

}



void  CDepthBasics::setPixel(int markpixelx ,int markpixely) {
	this->markpixelx = markpixelx;
	this->markpixely = markpixely;
}



void CDepthBasics::on_trackbar(int pos, std::vector<vector<Point>> & contours, cv::Mat & cv16Img)
{
	// תΪ��ֵͼ
	cvThreshold(g_pGrayImage, g_pBinaryImage, pos, 255, CV_THRESH_BINARY);
	// ��ʾ��ֵͼ

	char* pstrWindowsBinaryTitle = "My binary picture";

	cvShowImage(pstrWindowsBinaryTitle, g_pBinaryImage);

	//for test//




	cv::Mat MbinaryMat(g_pBinaryImage, 0);

	findContours(MbinaryMat, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);


	cv::Mat MyMat = cv::Mat::zeros(424, 512, CV_8UC1);

	for (int i = 0; i < contours.size(); i++) {
		cv::Mat contour_mat = cv::Mat(contours[i]);

		float m = contourArea(contour_mat);

		if (contourArea(contour_mat) > 100) {
			for (int k = 0; k < contours[i].size(); k++) {
				MyMat.at<uchar>(contours[i][k].y, contours[i][k].x) = 255;
			}
		}
	}

	drawContours(MyMat, contours, 0, Scalar(255), CV_FILLED);
	imshow("savedGrayMat", MyMat);

	//for test//
}


//�����ֵ��������//
void CDepthBasics::depthRevise(cv::Mat & cv16Img, std::vector<Point> & srcContours,std::vector<Point> & contours) 
{
	//test//
	//��������//

	std::vector<int> vdelPointIndex;//��Ҫ��ɾ���ĵ�����//  
	
	
	for (int i = 0; i < contours.size(); i++)
	{


		//�ڴ˼�p�����ڱ仯��,��Ҫ�õ�����ĵ�����ֵ//
		cv::Point2f srcPoint = contours[i];

		//��ȡ��������ص�//
		
		cv::Point2f pCenter;   //�洢һ�����ı�־��//

		std::vector<cv::Point2f> pNeighbour;
		cv::Point2f p = contours[i];
		getValidInnerPoint(cv16Img, srcContours, p);
		//��ȡ��������ص�//

		//�ڽ�����//
		pNeighbour = getNeighbour(p);
		//�ڽ�����//

		//��ȡ�����ص����Ч���ֵ//


		/*
		//test:�۲����ֵ�Ƿ��б任//
		float preDepth = cv16Img.at<USHORT>(p.y, p.x);
		//test:�۲����ֵ�Ƿ��б任//
		*/

		bool  bIsavgDepthValid = avgDepth(p, srcContours, cv16Img);


		/*
		float afterDepth1 = cv16Img.at<USHORT>(p.y, p.x);
		*/


		if (bIsavgDepthValid) {
			cv16Img.at<USHORT>(srcPoint.y, srcPoint.x) = cv16Img.at<USHORT>(p.y, p.x);
		}

		//test:�۲����ֵ�Ƿ��б任//


		std::vector<int>vNeighbour;
		vNeighbour.resize(100);
		int iLayer = 0;

		pCenter = p;


		//�涨һ����־λ�����ﵽ�ñ�־λʱ��whileѭ���˳�����������Ҳ���趨Ϊ��Ч��//
		int Mark = 0;   //���ñ�־λ����1,��ʾ���������ѭ��ʱ�Զ��˳�ѭ��//

		while (!bIsavgDepthValid && Mark ==0) {
			p = (pNeighbour[vNeighbour[iLayer]]);
			vNeighbour[iLayer] = vNeighbour[iLayer] + 1;

			//����õ�����ܶ�û�кõ�//
			if (vNeighbour[iLayer] == 8) {
				if (iLayer > 0 ) {
					if (vNeighbour[iLayer - 1] == 8) {
						Mark = 1;
					} 
					
					if (Mark == 0) {
						vNeighbour[iLayer] = 0;
						
						p = pCenter;
						pNeighbour = getNeighbour(p);
						p = pNeighbour[vNeighbour[iLayer - 1]];

						vNeighbour[iLayer - 1] = vNeighbour[iLayer - 1] + 1;

						pNeighbour = getNeighbour(p);
						p = pNeighbour[vNeighbour[iLayer]];
						vNeighbour[iLayer] = vNeighbour[iLayer] + 1;
					}

				}
				else 
				{
					vNeighbour[iLayer] = 0;
					iLayer = iLayer + 1;

					p = (pNeighbour[vNeighbour[iLayer - 1]]);
					pNeighbour = getNeighbour(p);
					vNeighbour[iLayer - 1] = vNeighbour[iLayer - 1] + 1;

				}

			}
			//����õ�����ܶ�û�кõ�//
			if (Mark == 0) {
				if (p.x > 0 && p.x < 512 && p.y > 0 && p.y < 424) {
					if (cv16Img.at<USHORT>(p.y, p.x) > 0) {
						getValidInnerPoint(cv16Img, srcContours, p);
						bIsavgDepthValid = avgDepth(p, srcContours, cv16Img);
					}
				}

				else {
					bIsavgDepthValid = false;
				}


			}
		}
		//��ȡ�����ص����Ч���ֵ//

		if (Mark == 1) {
			vdelPointIndex.push_back(i);
		}

		else {
			cv16Img.at<USHORT>(srcPoint.y, srcPoint.x) = cv16Img.at<USHORT>(p.y, p.x);
		}
	}

	std::vector<cv::Point> vCopyContours;

	if (vdelPointIndex.size() > 0) {
		for (int i = 0; i < contours.size(); i++) {
			for (int k = 0; k < vdelPointIndex.size(); k++) {
				if (i != vdelPointIndex[k]) {
					vCopyContours.push_back(contours[i]);
					break;
				}
			}
		}
	}
	else {
		vCopyContours = contours;
	}

	contours = vCopyContours;
	
	


		//test//
		//�����ֵ���в��ԣ����Ƿ��л���//

	    /*
		std::fstream out20("D:\\checkDepth.txt", std::ios::app);
		for (int i = 0; i < vOridinal.size(); i++) {
			out20 << vOridinal[i] << std::endl;
		}
		out20 << std::endl;
		out20 << std::endl;
		*/


		//�����ֵ���в��ԣ����Ƿ��л���//
		//test//

	
	//���������//

	//��������//
	//test//
}

//����ʶ���������ȷ�������ĵ��Ƿ��Ǳ߽��,ͬʱҪ����ĵ�Ϊ�ڲ���//
bool CDepthBasics::isBorderOrOut(const cv::Mat & cv16Img, const std::vector<Point> & contours, const cv::Point & p) {
	if (cv16Img.at<USHORT>(p.y, p.x) < 10)
		return true;
	for (int i = 0; i < contours.size(); i++) {
		if (p == contours[i])
			return true;
	}
	return false;
}



//ͨ�������ĵ㣬�Ըõ����Ƚ�������//

bool CDepthBasics::avgDepth(const cv::Point2f & p, const std::vector<Point> & srcContours, cv::Mat & cv16Img)
{
	USHORT depth1 = 0, depth2 = 0, depth3 = 0, depth4 = 0, depth5 = 0, depth6 = 0, depth7 = 0, depth8 = 0, depth9 = 0;

	cv::Point2f  pLeft, pRight, pUp, pDown, pLeftUP, pLeftDown, pRightUp, pRightDown;


	float depth = 0;
	int Count = 1;

	std::vector<cv::Point3f> vOridinal;     //for test :�ڼ���ƽ��ֵ�Ĺ������Ƿ���ڻ���//

		
		pLeft = cv::Point2f(p.x - 1, p.y);
		pRight = cv::Point2f(p.x + 1, p.y);
		pUp = cv::Point2f(p.x, p.y - 1);
		pDown = cv::Point2f(p.x, p.y + 1);


		//�Խ��߷���//
		pLeftUP = cv::Point2f(p.x - 1, p.y - 1);
		pLeftDown = cv::Point2f(p.x - 1, p.y + 1);
		pRightUp = cv::Point2f(p.x + 1, p.y - 1);
		pRightDown = cv::Point2f(p.x + 1, p.y + 1);
		//�Խ��߷���//

		depth1 = cv16Img.at<USHORT>(p.y, p.x);
		depth = depth + depth1;
		vOridinal.push_back(Point3f(p.x, p.y, depth1));

		if (pLeft.x > 0 && pLeft.x < 512 && pLeft.y >0 && pLeft.y < 424) {
		    if (!isBorderOrOut(cv16Img, srcContours, pLeft)) {
			
				depth2 = cv16Img.at<USHORT>(pLeft.y, pLeft.x);
				depth = depth + depth2;
				Count++;
				vOridinal.push_back(Point3f(pLeft.x, pLeft.y, depth2));    //for test :�ڼ���ƽ��ֵ�Ĺ������Ƿ���ڻ���//
			}



		}


		if (pRight.x > 0 && pRight.x < 512 && pRight.y >0 && pRight.y < 424) {
		    if (!isBorderOrOut(cv16Img, srcContours, pRight)) {
			
				depth3 = cv16Img.at<USHORT>(pRight.y, pRight.x);
				depth = depth + depth3;
				Count++;

				vOridinal.push_back(Point3f(pRight.x, pRight.y, depth3));  //for test :�ڼ���ƽ��ֵ�Ĺ������Ƿ���ڻ���//
			}


		}


		if (pUp.x > 0 && pUp.x < 512 && pUp.y >0 && pUp.y < 424) {
		   if (!isBorderOrOut(cv16Img, srcContours, pUp)) {
				depth4 = cv16Img.at<USHORT>(pUp.y, pUp.x);
				depth = depth + depth4;
				Count++;

				vOridinal.push_back(Point3f(pUp.x, pUp.y, depth4));  //for test :�ڼ���ƽ��ֵ�Ĺ������Ƿ���ڻ���//
			}


		}


		if (pDown.x > 0 && pDown.x < 512 && pDown.y >0 && pDown.y < 424) {
			if (!isBorderOrOut(cv16Img, srcContours, pDown)) {
				depth5 = cv16Img.at<USHORT>(pDown.y, pDown.x);
				depth = depth + depth5;
				Count++;

				vOridinal.push_back(Point3f(pDown.x, pDown.y, depth5));  //for test :�ڼ���ƽ��ֵ�Ĺ������Ƿ���ڻ���//
			}
		}



		//��ƽ��ֵ��8���������Count����//   
		if (Count < 3) {

			if (pLeftUP.x > 0 && pLeftUP.x < 512 && pLeftUP.y >0 && pLeftUP.y < 424) {
				if (!isBorderOrOut(cv16Img, srcContours, pLeftUP)) {
					depth6 = cv16Img.at<USHORT>(pLeftUP.y, pLeftUP.x);
					depth = depth + depth6;
					Count++;
					vOridinal.push_back(Point3f(pLeftUP.x, pLeftUP.y, depth6));  //for test :�ڼ���ƽ��ֵ�Ĺ������Ƿ���ڻ���//

				}
			}

			if (pLeftDown.x > 0 && pLeftDown.x < 512 && pLeftDown.y >0 && pLeftDown.y < 424) {
				if (!isBorderOrOut(cv16Img, srcContours, pLeftDown)) {
					depth7 = cv16Img.at<USHORT>(pLeftDown.y, pLeftDown.x);
					depth = depth + depth7;
					Count++;
					vOridinal.push_back(Point3f(pLeftDown.x, pLeftDown.y, depth7)); //for test :�ڼ���ƽ��ֵ�Ĺ������Ƿ���ڻ���//

				}
			}


			if (pRightUp.x > 0 && pRightUp.x < 512 && pRightUp.y >0 && pRightUp.y < 424) {
				if (!isBorderOrOut(cv16Img, srcContours, pRightUp)) {
					depth8 = cv16Img.at<USHORT>(pRightUp.y, pRightUp.x);
					depth = depth + depth8;
					Count++;

					vOridinal.push_back(Point3f(pRightUp.x, pRightUp.y, depth8));  //for test :�ڼ���ƽ��ֵ�Ĺ������Ƿ���ڻ���//
				}
			}


			if (pRightDown.x > 0 && pRightDown.x < 512 && pRightDown.y >0 && pRightDown.y < 424) {
				if (!isBorderOrOut(cv16Img, srcContours, pRightDown)) {
					depth9 = cv16Img.at<USHORT>(pRightDown.y, pRightDown.x);
					depth = depth + depth9;
					Count++;
					vOridinal.push_back(Point3f(pRightDown.x, pRightDown.y, depth9));  //for test :�ڼ���ƽ��ֵ�Ĺ������Ƿ���ڻ���//
				}
			}

		}
		//��ƽ��ֵ��8���������Count����//


		if (Count >2)
		{
			std::vector<float> vOridinalDepth;
			for (int i = 0; i < vOridinal.size(); i++) {
				vOridinalDepth.push_back(vOridinal[i].z);
			}

			cv::Point2f pMaxDepth = getMaxDepth(vOridinalDepth);
			float fmaxdepth = pMaxDepth.y;
			int iMaxIndex = (int)(pMaxDepth.x);

			cv::Point2f pMinDepth = getMinDepth(vOridinalDepth);
			float fmindepth = pMinDepth.y;
			int iMinIndex = (int)(pMinDepth.x);




			//����ͨ��bool���ж��Ƿ���Ҫ������vOridinalDepth���е����� ��bool Ϊfalseʱ,�������Сֵ�Ĳ�����ֵ������Χ�ڣ�������Ϊ�����������Ǻ����ݣ�����Ҫ�����������ô����ƽ��ֵ��������������ݽ�������//
			float avgDepth = depth / Count;
			while (elimateBadDepth(pMaxDepth, pMinDepth, avgDepth, vOridinalDepth) && (vOridinalDepth.size()>2)) {
				pMaxDepth = getMaxDepth(vOridinalDepth);
				pMinDepth = getMinDepth(vOridinalDepth);
				avgDepth = 0;
				for (int i = 0; i < vOridinalDepth.size(); i++) {
					avgDepth += vOridinalDepth[i];
				}
				avgDepth = avgDepth / (vOridinalDepth.size());
			}

			if (vOridinalDepth.size()>2) {

				cv16Img.at<USHORT>(p.y,p.x) =  avgDepth;
				return true;
			}

			else {
				if (Count == 1)
					return false;
				else
					if (fabs(vOridinal[0].z - vOridinal[0].z) > 70) {
						return false;
					}

					else {
						return true;
					}
						
			}
		}

		else {

				if (Count == 1)
					return false;
				else {
					if (fabs(vOridinal[0].z - vOridinal[0].z) > 70) {
						return false;
					}
					else {
						return true;
					}
				}

			}	
}


cv::Point2f CDepthBasics::getMaxDepth(const std::vector<float>  & vOridinalDepth) {
	int k = 0;
	float fMaxDepth = 0;
	for (int i = 0; i < vOridinalDepth.size(); i++) {
		if (vOridinalDepth[i]>fMaxDepth) {
			fMaxDepth = vOridinalDepth[i];
			k = i;
		}
	}
	
	return cv::Point2f(k, fMaxDepth);
}

cv::Point2f CDepthBasics::getMinDepth(const std::vector<float>  & vOridinalDepth) {
	int k = 0;
	float fMinDepth = 10000;
	for (int i = 0; i < vOridinalDepth.size(); i++) {
		if (vOridinalDepth[i]< fMinDepth) {
			fMinDepth = vOridinalDepth[i];
			k = i;
		}
	}
	return cv::Point2f (k, fMinDepth);
}


//�޵������ݵĺ���//

bool CDepthBasics::elimateBadDepth(const cv::Point2f  & pMaxDepth, const cv::Point2f  & pMinDepth, const float  & avgDepth, std::vector<float>  & vOridinalDepth)
{



	std::vector<float>   vCopyOridinalDepth;
	float fmaxdepth = pMaxDepth.y;
	float fmindepth = pMinDepth.y;
	int delIndex = 0;


	if (fabs(fmaxdepth - fmindepth) < 70) {
		return false ;
	}

	else {
		if (fabs(fmaxdepth - avgDepth) > fabs(avgDepth - fmindepth)) {
			delIndex = (int)(pMaxDepth.x);
		}
		else {
			delIndex = (int)(pMinDepth.x);
		}

		for (int i = 0; i < vOridinalDepth.size(); i++) {
			if (i != delIndex) {
				vCopyOridinalDepth.push_back(vOridinalDepth[i]);
			}
		}
	}

	vOridinalDepth = vCopyOridinalDepth;
}




//���ǵ��߽������ֵ���ȶ������߽��������е��ڲ����滻���߽��//
void CDepthBasics::getValidInnerPoint(const cv::Mat & cv16Img, const std::vector<Point> & srcContours, cv::Point2f & p)
{
	cv::Point2f  pLeft, pRight, pUp, pDown, pLeftUP, pLeftDown, pRightUp, pRightDown;
	    
	    //�ڽ�����//
		pLeft = cv::Point2f(p.x - 1, p.y);
		pRight = cv::Point2f(p.x + 1, p.y);
		pUp = cv::Point2f(p.x, p.y - 1);
		pDown = cv::Point2f(p.x, p.y + 1);
		//�ڽ�����//

		//�Խ��߷���//
		pLeftUP = cv::Point2f(p.x - 1, p.y - 1);
		pLeftDown = cv::Point2f(p.x - 1, p.y + 1);
		pRightUp = cv::Point2f(p.x + 1, p.y - 1);
		pRightDown = cv::Point2f(p.x + 1, p.y + 1);
		//�Խ��߷���//


		std::vector<Point2f> pneighbour;
		pneighbour.push_back(pLeft);
		pneighbour.push_back(pRight);
		pneighbour.push_back(pUp);
		pneighbour.push_back(pDown);


		//�����ĸ������еĵ㣬Ϊ��ȡ��Ч���ڲ���//
		pneighbour.push_back(pLeftUP);
		pneighbour.push_back(pLeftDown);
		pneighbour.push_back(pRightUp);
		pneighbour.push_back(pRightDown);
		//�����ĸ������еĵ㣬Ϊ��ȡ��Ч���ڲ���//


		int m = 0;    //pneighbour���е�����//

		//�����ڲ����滻���߽��//
		for (m = 0; m < 4; m++) {
			cv::Point pTest = pneighbour[m];

			if (!isBorderOrOut(cv16Img, srcContours, pTest)) {
				p = pneighbour[m];
				break;
			}
		}

		/*
		//test//
		//�Ƿ����ĸ���Ҫô�Ǳ߽�㣬Ҫô���ⲿ��//
		std::fstream out17("D:\\fourneighbour.txt",std::ios::app);
		for (int i = 0; i < pneighbour.size(); i++) {
		out17 << pneighbour[i].x << " " << pneighbour[i].y <<" " <<cv16Img.at<USHORT>(pneighbour[i].y, pneighbour[i].x) << std::endl;
		}

		out17 << std::endl;
		out17 << std::endl;


		std::fstream out18("D:\\auxiliaryfourneighbour.txt", std::ios::app);
		for (int i = 0; i < srcContours.size(); i++) {
		out18 << srcContours[i].x << " " << srcContours[i].y << " " << cv16Img.at<USHORT>(srcContours[i].y, srcContours[i].x) << std::endl;
		}

		out18 << std::endl;
		out18 << std::endl;


		//����M����ֵ//
		std::fstream out19("D:\\auxiliaryfourneighbourGetM.txt", std::ios::app);
		out19 << m << std::endl;


		//�Ƿ����ĸ���Ҫô�Ǳ߽�㣬Ҫô���ⲿ��//

		//test result:�����ĸ��ڽ���Ҫô��ʶ�������⣬Ҫôͬ��Ϊ�߽�㣬��˿��Կ��ǽ����������8�����������������߽��Ĵ���//

		//test//
		*/

		assert(m < 8);  //�����ʱ�򲻻����ʾerror���������ʱ�����ʾerror//
	
}

std::vector<cv::Point2f> CDepthBasics::getNeighbour(const cv::Point2f & p) {
	cv::Point2f pLeft, pRight, pUp, pDown, pLeftUP,pLeftDown,pRightUp,pRightDown;
	std::vector<cv::Point2f> pNeighbour;
	//�ڽ�����//
	pLeft = cv::Point2f(p.x - 1, p.y);
	pRight = cv::Point2f(p.x + 1, p.y);
	pUp = cv::Point2f(p.x, p.y - 1);
	pDown = cv::Point2f(p.x, p.y + 1);

	pLeftUP = cv::Point2f(p.x - 1, p.y - 1);
	pLeftDown = cv::Point2f(p.x - 1, p.y + 1);
	pRightUp = cv::Point2f(p.x + 1, p.y - 1);
	pRightDown = cv::Point2f(p.x + 1, p.y + 1);

	pNeighbour.push_back(pLeft);
	pNeighbour.push_back(pRight);
	pNeighbour.push_back(pUp);
	pNeighbour.push_back(pDown);
	pNeighbour.push_back(pLeftUP);
	pNeighbour.push_back(pLeftDown);
	pNeighbour.push_back(pRightUp);
	pNeighbour.push_back(pRightDown);
	//�ڽ�����//
	return pNeighbour;
}


//�ڵ����߸�������ǰ���Ǳ����趨һ����ֵ������ƽ����ľ��������ֵ��Χ��ʱ����Ϊ��������վ��һ��ģ�����ʹ�øú���//
//��������վ����Ļ��һ���ʱ�����ƿ��ܲ���ˮƽ����ʱ���������Ϣ���ٿ��У������������ص����Ϣ����Ϊ��ʱ���е����ص�λ��һ�࣬������һ�������Զ�ĵ���������Ƶļ�˵�//
//Input��opencv��ȡ��������//
//output:���Ƶ�ָ���//

bool CDepthBasics::getSummitPoint(const float  & threshold, const std::vector<cv::Point>  & srcContours, cv::Point  & fSummitPoint,const cv::Mat & cv16img) {


	//��Ҫ��srcContours�еĵ�ת��������Ļ�����ͶӰ����ʽ������������������Ļ�ϵ�ͶӰ����Ļ��Ե������Ϊ��������վ����Ļ��һ���//
	//1��תΪ��Ļ����//

	V3f ImgKinectPos, ImgProjectPos, ImgScreenPos;
	std::vector<V3f> vImgKinectPos;
	for (int i = 0; i < srcContours.size(); i++) {
		ImgKinectPos = getKinectPos(V3f(srcContours[i].x, srcContours[i].y, cv16img.at<USHORT>(srcContours[i].y, srcContours[i].x)));
		ImgProjectPos = getProjectPos(ImgKinectPos);
		vImgKinectPos.push_back(ImgProjectPos);

		//����һ���õ��Kinect�����xֵ��ͶӰ��ƽ���ϵ�x�����Ƿ����//

	}


	//for test: ���ñ�־λ//
	enum eDir {
		left,
		right,
	};

	eDir ehordir, evertdir, esummitdir;
	//for test: ���ñ�־λ//

	int iMinImgKinectx, iMaxImgKinectx;   //ʶ�������x��y�����ϵ������С���������//
	int iMaxIndex, iMinIndex;       //��Ӧx,y�����������С���ص�ĵ�ָ��//

	float fMinDistx = 0, fMaxDistx = 0, fMinDisty = 0, fMaxDisty = 0;

	float fHorMinDist, fVertMinDist;  //��ȡˮƽ����ʹ�ֱ�������߽���С��dist//

	iMinImgKinectx = 10000;
	iMaxImgKinectx = -10000;
	for (int i = 0; i < vImgKinectPos.size(); i++) {
		if (iMinImgKinectx > vImgKinectPos[i].x) {
			iMinImgKinectx = vImgKinectPos[i].x;
			iMinIndex = i;
		}



		if (iMaxImgKinectx < vImgKinectPos[i].x) {
			iMaxImgKinectx = vImgKinectPos[i].x;
			iMaxIndex = i;
		}

	}

	//��ȡ��x���ط����ϵ���С������ص���x�߽�ľ���//
	fMinDistx = abs(iMinLimitx - iMinImgKinectx);
	fMaxDistx = abs(iMaxLimitx - iMaxImgKinectx);

	//����ֵ�����жϣ�ȷ���ú����Ƿ���ã������ã��ж����Ƶı߽�����//


	std::vector<float> vMinDist;
	vMinDist.push_back(fMinDistx);
	vMinDist.push_back(fMaxDistx);

	float fMinDist = vMinDist[0];
	int iGlobalMinIndex = 0;
	for (int i = 1; i < vMinDist.size(); i++) {
		if (vMinDist[i] < fMinDist) {
			fMinDist = vMinDist[i];
			iGlobalMinIndex = i;
		}
	}

	if (threshold < fMinDist) {
		return false;
	}
	esummitdir = (eDir)(iGlobalMinIndex);   
	//����ֵ�����жϣ�ȷ���ú����Ƿ���ã������ã��ж����Ƶı߽�����//




	switch (esummitdir) {
	case left: {
		fSummitPoint = srcContours[iMaxIndex];
		break;
	}

	case right: {
		fSummitPoint = srcContours[iMinIndex];
		break;
	}

	default: {
		fSummitPoint = cv::Point2f(0, 0);
		break;
	}
	}

	if (fSummitPoint == cv::Point(0, 0)) {
		return false;
	}

	else {
		return true;
	}
}




//1�����Ƚ��з��࣬��ȡ�������2��Ȼ��ȡ���������ƽ����ȣ�����Ƚϴ��Ϊ�ֱ۰�����Ƚ�С��Ϊָ�����3��Ȼ�����ָ������ֱ۰������ĵľ��룬���Ϊָ��//
//���ڻ�ȡ����ֻ��3�������Σ�ֻ��Ҫ�߲���1��2�������������İ���ʣ�µ�Ϊָ���//
//���ڻ�ȡ������ֻ�����������Σ�ȡȡ�����Ͻ��ĵ�//
//���ڻ�ȡ������ֻ��һ�������Σ�ֱ��ʹ��//
cv::Point2f CDepthBasics::getSummitPoint(const std::vector<cv::Point> & srcContours, const cv::Mat & cv16Img, std::vector<cv::Point> & contours) {
	int k = 2;
	float threshold = 10;
	std::vector<std::vector<cv::Point>> vGroups;


	if (contours.size() == 1) {
		return contours[0];
	}

	else if (contours.size() == 2) {
		float pixely1 = contours[0].y;
		float pixely2 = contours[1].y;
		if (pixely1 > pixely2) {
			return contours[0];
		}
		else {
			return contours[1];
		}
	}


	else {

		vGroups = getGroup(k, contours, threshold);

		if (contours.size() == 3) {
			if (vGroups[0].size() == 1) {
				return vGroups[0][0];
			}
			else {
				return vGroups[1][0];
			}
		}

		else {
			std::vector<float> vPixely;
			float pixely = 0;
			for (int i = 0; i < vGroups.size(); i++) {
				pixely = getAvgPixely(vGroups[i]);
				vPixely.push_back(pixely);
			}

			//��Kinect���°�װʱ������װ��Kinect������������Ļ��������Ƽ��pixel�ϴ�//
			//�����������������Σ�ѡ��pixely��С�İ���Ϊ�޳�����pixely���İ���Ϊ���������pixely��С�İ�Ϊ��׼����pixely���İ��У��õ����Ƽ�˵�//
			//Ӧ��ȡ��������pixely��С�İ�����ϴ�ĵ���Ϊ���Ƶļ�˵�//
			float fMinPixely = 10000, fMaxPixely = -10000;
			int iMinIndex = 0, iMaxIndex = 0;
			for (int i = 0; i < vPixely.size(); i++) {
				if (fMinPixely >vPixely[i]) {
					fMinPixely = vPixely[i];
					iMinIndex = i;
				}
				if (fMaxPixely < vPixely[i]) {
					fMaxPixely = vPixely[i];
					iMaxIndex = i;
				}
			}
			//��ȡ��С�������ģ�����С���㵽�����ĵľ������Ϊ�жϼ�˵�ı�׼//  //�����������С�ĵ㣬��С����������ĵ�//


			cv::Point2f referp = getOptimalPoint(vGroups[iMinIndex]);
			float fMaxDist = 0;
			int iMaxDistIndex = 0;
			float fDist = 0;

			//Ϊ�������㷨��������,�����Ժ�k>2�����Σ��ر�ĵ���С���ĵ���ĿΪ1ʱ���ر�����������һ��//
			if (vGroups[iMaxIndex].size() == 1) {
				return vGroups[iMaxIndex][0];
			}

			else {
				for (int i = 0; i < vGroups[iMaxIndex].size(); i++) {
					cv::Point2f p(referp.x - vGroups[iMaxIndex][i].x, referp.y - vGroups[iMaxIndex][i].y);
					fDist = sqrt(p.x * p.x + p.y *p.y);
					if (fMaxDist < fDist) {
						fMaxDist = fDist;
						iMaxDistIndex = i;
					}
				}
			}


			return vGroups[iMaxIndex][iMaxDistIndex];
		}
	}

}


std::vector<std::vector<cv::Point>>  CDepthBasics::getGroup(const int & k, const std::vector<cv::Point>  & srcContours, const float  & threshold) {
	std::vector<std::vector<cv::Point>> vGroups;
	vGroups.resize(k);
	std::vector<cv::Point> vMarkPoint;
	vMarkPoint.resize(k);
	//���ó�ʼ��center��//
	for (int i = 0; i < k; i++) {
		vMarkPoint[i] = srcContours[i];
	}
	//���ó�ʼ��center��//

	std::vector<cv::Point> vLastPoint;
	vLastPoint = vMarkPoint;
	std::vector<float> vCenterChangeDist;
	vCenterChangeDist.resize(k);
	for (int i = 0; i < k; i++) {
		vCenterChangeDist[i] = 10000;
	}

	float fMinCenterChangeDist = getMin(vCenterChangeDist);

	while (fMinCenterChangeDist > threshold) {
		UpdateGroup(vMarkPoint, srcContours, vGroups);
		for (int i = 0; i < k; i++) {
			vMarkPoint[i] = getOptimalPoint(vGroups[i]);
			cv::Point2f pDist(vMarkPoint[i] - vLastPoint[i]);
			vCenterChangeDist[i] = sqrt(pDist.x * pDist.x + pDist.y * pDist.y);
			vLastPoint[i] = vMarkPoint[i];
		}

		fMinCenterChangeDist = getMin(vCenterChangeDist);
	}
	return vGroups;

}

//belong to K-Means Algorithm//
//��srcContours��Ĺ���//

void CDepthBasics::UpdateGroup(const std::vector<cv::Point> & vMarkPoint, const std::vector<cv::Point>  & srcContours, std::vector<std::vector<cv::Point>> &vGroups) {

	float fMinDist = 10000;
	int iMinIndex = 0;
	float fdist = 0;
	for (int i = 0; i < srcContours.size(); i++) {
		for (int j = 0; j<vMarkPoint.size(); j++) {
			cv::Point2f p((srcContours[i].x - vMarkPoint[j].x), (srcContours[i].y - vMarkPoint[j].y));
			fdist = sqrt(p.x * p.x + p.y * p.y);
			if (fdist < fMinDist) {
				fMinDist = fdist;
				iMinIndex = j;
			}
		}
		fMinDist = 10000;
		vGroups[iMinIndex].push_back(srcContours[i]);
	}
}

//��srcContours��Ĺ���//
//belong to K-Means Algorithm//


float CDepthBasics::getMin(std::vector<float> vDist) {

	float fMindist = 10000;
	for (int i = 0; i < vDist.size(); i++) {
		if (vDist[i] < fMindist) {
			fMindist = vDist[i];
		}
	}
	return fMindist;
}


//belong to K-Means Algorithm//
//��ȡ�����е�ľ������С�ĵ�//
cv::Point CDepthBasics::getOptimalPoint(const std::vector<cv::Point> & srcContours) {
	float favgx = 0, favgy = 0;
	cv::Point2f optimalPoint;
	for (int i = 0; i < srcContours.size(); i++) {
		favgx = favgx + srcContours[i].x;
		favgy = favgy + srcContours[i].y;
	}

	optimalPoint.x = (favgx / (srcContours.size()));
	optimalPoint.y = (favgy / (srcContours.size()));
	return optimalPoint;
}
//��ȡ�����е�ľ������С�ĵ�//
//belong to K-Means Algorithm//


//belong to getSummitPoint//
//��ȡ��һ�����еı�Ե���ƽ�����//
float CDepthBasics::getAvgDepth(const cv::Mat & cv16Img, std::vector<cv::Point> & contours) {
	float depth = 0;
	for (int i = 0; i < contours.size(); i++) {
		depth = depth + cv16Img.at<USHORT>(contours[i].y, contours[i].x);
	}
	depth = depth / (contours.size());
	return depth;
}
//��ȡ��һ�����еı�Ե���ƽ�����//
//belong to getSummitPoint//

float CDepthBasics::getAvgPixely(std::vector<cv::Point> & contours) {
	float avgPixely = 0;
	for (int i = 0; i < contours.size(); i++) {
		avgPixely = avgPixely + contours[i].y;
	}
	avgPixely = avgPixely / (contours.size());
	return avgPixely;
}

//����㵽ƽ��ľ���//
void CDepthBasics::CalDistFromPointToPlane(const V3f &p, const Plane3f & plane, float dist) 
{
	V3f intersetPos;
	V3f dir = plane.normal;
	Line3f line;
	line.dir = dir;
	line.pos = p;
	plane.intersect(line, intersetPos);
	dist = (p - intersetPos).length();
}


//���ں�ǰһ֡�����ƽ���ƥ��//
bool  CDepthBasics::getMarchLastHandPos(const std::vector<sAction>  & vLastHand, const cv::Point3f  & kinectPos, const float & threshold, int  & index)
{
	
	float fMinDist = 10000;
	float fDist;
	int iMinIndex = 0;
	for (int i = 0; i < vLastHand.size(); i++) {
		cv::Point3f pDist = cv::Point3f(vLastHand[i].worldPos.x - kinectPos.x, vLastHand[i].worldPos.y - kinectPos.y, vLastHand[i].worldPos.z - kinectPos.z);
		fDist = sqrt(pDist.x * pDist.x + pDist.y * pDist.y + pDist.z * pDist.z);
		if (fMinDist > fDist) {
			fMinDist = fDist;
			iMinIndex = i;
		}
	}

	if (fMinDist < threshold) {
		index = iMinIndex;
		return true;
	}

	else {
		return false;
	}
}


//����true��ʾ�ڱ�Ե�����Ϊfalse��ʾ��ʶ��������//
bool CDepthBasics::IsInOrOutOfZone(const V3f  & KinectPos, const float & fSpeedLength, V3f  vfPospre, V3f vfPosAfter)
{

	std::vector<float> vPreposDist;
	std::vector<float> vAfterposDist;
	vPreposDist.resize(5);
	vAfterposDist.resize(5);
	std::vector<int> vDir; //���ٶȳ���÷���ʱ�򣬶�Ӧ�������ڵ�ֵ��Ϊ1��������Ϊ0//
	std::vector<float> fPosDist;
	fPosDist.resize(5);
	//�����ƽ�����ϸ��//
	Plane3f p1, p2, p3, p4, p5;

	// vKinectPos��ǵ��ĸ��㣻vdistMarkPoint:�������ĸ��� 
	V3f vMark1, vMark2, vMark3, vMark4, vDistMark1, vDistMark2, vDistMark3, vDistMark4;

	vMark1 = V3f(vKinectPos[0].x, vKinectPos[0].y, vKinectPos[0].z);
	vMark2 = V3f(vKinectPos[1].x, vKinectPos[1].y, vKinectPos[1].z);
	vMark3 = V3f(vKinectPos[2].x, vKinectPos[2].y, vKinectPos[2].z);
	vMark4 = V3f(vKinectPos[3].x, vKinectPos[3].y, vKinectPos[3].z);

	vDistMark1 = V3f(vdisMarkPoint[0].x, vdisMarkPoint[0].y, vdisMarkPoint[0].z);
	vDistMark2 = V3f(vdisMarkPoint[1].x, vdisMarkPoint[1].y, vdisMarkPoint[1].z);
	vDistMark3 = V3f(vdisMarkPoint[2].x, vdisMarkPoint[2].y, vdisMarkPoint[2].z);
	vDistMark4 = V3f(vdisMarkPoint[3].x, vdisMarkPoint[3].y, vdisMarkPoint[3].z);

	p1 = Plane3f(vDistMark1, vDistMark2, vDistMark3);
	p2 = Plane3f(vMark1, vDistMark1, vDistMark4);
	p3 = Plane3f(vMark2, vDistMark2, vMark3);
	p4 = Plane3f(vDistMark3, vDistMark3, vMark4);
	p5 = Plane3f(vMark1, vDistMark1, vMark2);


	std::vector<Plane3f> vPlane;
	vPlane.resize(5);

	for (int i = 0; i < vPlane.size(); i++) {
		CalDistFromPointToPlane(KinectPos, vPlane[i], fPosDist[i]);
	}

	if (fSpeedLength > 0.1) {
		for (int i = 0; i < vPlane.size(); i++) {
			CalDistFromPointToPlane(vfPospre, vPlane[i], vPreposDist[i]);
			CalDistFromPointToPlane(vfPosAfter, vPlane[i], vAfterposDist[i]);
			if (vPreposDist[i] > vAfterposDist[i]) {
				vDir.push_back(1);
			}
			else {
				vDir.push_back(0);
			}
		}
	}

	else {
		vDir.resize(vPlane.size());
		for (int i = 0; i < vPlane.size(); i++) {
			vDir[i] = 0;
		}
	}

	//�����һ�����޷�ʶ�𵽣��жϸõ��Ƿ�����Ϊ����ʶ�����������//


	for (int i = 0; i < vDir.size(); i++) {
		if (vDir[i] == 1) {
			if (fPosDist[i] < fOutThreshold) {
				return true;

			}

		}
	}
	return false;
	//�����һ�����޷�ʶ�𵽣��жϸõ��Ƿ�����Ϊ����ʶ�����������//
	
}

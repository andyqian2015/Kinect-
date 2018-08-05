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
		Update();


		
		int k = 0;
		if (depthData.size() > 0) {
			cv::Mat cv16Img(cDepthHeight, cDepthWidth, CV_16UC1);
			cv::Mat cv8Img(cDepthHeight, cDepthWidth, CV_8UC1);

			int t_ms = (FrameTime - m_nStartTime) / 1000;

			//for test check if the depth is right//
			
			for (int i = 0; i < 424; i++) {
				for (int j = 0; j < 512; j++) {
						cv16Img.at<USHORT>(i, j) = depthData[k];
					k++;
				}
			}
			
			//for test check if the depth is right//

			k = 0;


			Precalc16_fast(cv16Img, cv8Img);
			SeekHands(cv16Img, cv8Img);

			/*
			//test//
			//获取对边界裁剪后的图像深度值//
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
			//获取对边界裁剪后的图像深度值//
			//test//
			*/
			depthData.clear();
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

	//对手势运动的速度初始化//
	fHandPosThreshold = 100;


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

	V3f v1, v2, v3, v4,v_1, v_2, v_3, v_4;
	v1 = vKinectPos[0];
	v2 = vKinectPos[1];
	v3 = vKinectPos[2];
	v4 = vKinectPos[3];

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

	V3f vTestPixel(262, 142, 1820);
	V3f KinectPos = getKinectPos(vTestPixel);
	V3f vProjectPoint = getProjectPos(KinectPos);
	V3f vScreenPos = convertTo(vProjectPoint);
}

void CDepthBasics::ProcessDepth(INT64 nTime, const UINT16* pBuffer, int nWidth, int nHeight, USHORT nMinDepth, USHORT nMaxDepth)
{

	int pos = (depthPixely * nWidth + depthPixelx);

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


		WCHAR szStatusMessage[512];

		//show mouse depth//
		USHORT srcDepth = *(pBuffer + pos);
		//show mouse depth//

		StringCchPrintf(szStatusMessage, _countof(szStatusMessage), L" FPS = %0.2f    Time = %I64d  depth = %d\n", fps, (nTime - m_nStartTime), srcDepth);   //this time can be the timestamp//
		//	   ATLTRACE("%S\n",szStatusMessage);

	

		if (SetStatusMessage(szStatusMessage, 1000, false))
		{
			m_nLastCounter = qpcNow.QuadPart;
			m_nFramesSinceUpdate = 0;
		}
	}

	// Make sure we've received valid data

	std::fstream out5("D:\\ForthDepth.txt", std::ios::app);
	

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


		//tets:显示所有的凸点//
		/*
			
		std::vector<int> vPos;
		for (int i = 0; i < identPixel.size(); i++) {
			int pos = (identPixel[i].y * nWidth + identPixel[i].x);
			vPos.push_back(pos);
		}
		*/
		//tets:显示所有的凸点//
		
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


		while (pBuffer < pBufferEnd)
		{
			

			USHORT depth = *pBuffer;

			depthData.push_back(depth);


			// To convert to a byte, we're discarding the most-significant
			// rather than least-significant bits.
			// We're preserving detail, although the intensity will "wrap."
			// Values outside the reliable depth range are mapped to 0 (black).

			// Note: Using conditionals in this loop could degrade performance.
			// Consider using a lookup table instead when writing production code.
			BYTE intensity = static_cast<BYTE>((depth >= nMinDepth) && (depth <= nMaxDepth) ? (depth % 256) : 0);

			if (abs(iCurse - TestPos) < 2) {
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
	//先考虑一个手操作的情形,多手再进行扩充//
	std::vector<sAction> vCopyPreHands;
	vCopyPreHands = m_hands;
	m_hands.clear();

	//test//
	//利用二值图获取轮廓//
	g_pGrayImage = &IplImage(cv8img);

	//将灰度图转化成二值图//
	g_pBinaryImage = cvCreateImage(cvGetSize(g_pGrayImage), IPL_DEPTH_8U, 1);
	std::vector<vector<Point>> contours;
	on_trackbar(50, contours, cv16img);
	//for test//


	//	std::vector< std::vector<Point> > contours;
	//	cv::findContours(cv8img, contours, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE);  //call findContours to get contours//
#define HAND_LIKELY_AREA 100

	std::fstream out("D:\\act.txt", std::ios::app);
	std::fstream out1("D:\\pixel.txt", std::ios::app);
	for (unsigned i = 0; i < contours.size(); i++)
	{
		//只有在检测到轮廓时才会去求它的多边形，凸包集，凹陷集


		cv::Mat contour_mat = cv::Mat(contours[i]);

		//		drawContours(cv8img, contours, i, Scalar(255, 0, 0), 1, 8); //画出轮廓
		if (contourArea(contour_mat) > HAND_LIKELY_AREA)
		{   //比较有可能像手的区域

			//			CalcAveDepth(cv16img, contours[i]);  //对图像的边缘，通过这种方式剔除掉深度变换异常的点，获取到平均深度//


			//			cv::Mat cvDrawImg(cDepthHeight, cDepthWidth, CV_8UC1);


			cv::Point3f imgpt;

			cv::Point2f cen; float radius = 0;
			minEnclosingCircle(contours[i], cen, radius);

			//test//
			//利用轮廓最小外接圆圆心和轮廓的边界求值，确定手势的边界点//

			std::vector<Point> MyContours;

			float dist = 0, dist1 = 0;
			cv::Point pMyPoint;

			std::vector<int> oridinal;

			std::vector<int> vdelneighbourPoint;  //剔除掉的已经存在的在MyCotours中的邻域中的点//




			//对于有可能出现的，找到的较大值点总在已经找到的边界轮廓附近的情况，使得后面的非需要的边界轮廓点出现在视野中，采取的方法为，设定点的阈值//
			const int iPointthreshold = 30;
			while (MyContours.size() < 5 && (oridinal.size()< iPointthreshold)) {
				bool bAddVectorElement = true;
				int imaxindex = 0;
				for (int j = 0; j < contours[i].size(); j++) {
					int k = 0;
					bool flag = true;


					//剔除已取出的较大dist//
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



				oridinal.push_back(imaxindex);  //////////////////////////为简便起见，对于存在于MyCouters以及剔除掉的点都放在oridinal中
				dist = 0;
				for (int i = 0; i < MyContours.size(); i++) {
					cv::Point pMyContours = MyContours[i];
					if ((abs(pMyPoint.x - pMyContours.x) <= 10) && (abs(pMyPoint.y - pMyContours.y) <= 10)) {                         
						bAddVectorElement = false;
						break;
					}
				}

				//加一个判断语句，如果两点的像素离的比较近，只选择其中一个作为末端像素点//
				if (bAddVectorElement) {
					MyContours.push_back(pMyPoint);
				}
			}
			 

			/*
			//for test//
			//获取提取出来的五个边界点的像素坐标，打印出来//
			std::fstream out21("D:\\SurroundCircleBoundaryPoint.txt", std::ios::app);
			for (int i = 0; i < MyContours.size(); i++) {
				out21 << MyContours[i].x << " " << MyContours[i].y << std::endl;
			}
			out21 << std::endl;
			out21 << std::endl;
			*/



			//获取提取出来的五个边界点的像素坐标，打印出来//
			//for test//


			//test//
			//对获取的轮廓点的深度进行修正//


//			depthRevise(cv16img, contours[i], MyContours);     //delete for contemporary for test the K-means way//


			//对获取的轮廓点的深度进行修正//
			//test//


			/*
			//test//
			//用于记录获取opencv采集到的手势轮廓边缘，以及对应的深度值//
			std::fstream out9("D:\\contours.txt", std::ios::app);
			for (int i = 0; i < contours.size(); i++) {
				for (int k = 0; k < contours[i].size(); k++) {
					out9 << contours[i][k].x << " " << contours[i][k].y << " "<< cv16img.at<USHORT>(contours[i][k].y, contours[i][k].x) << std::endl;
				}
			}
			out9 << std::endl;
			out9 << std::endl;
			//用于获取opencv采集到的手势轮廓边缘//
			//test//
			*/


			/*
			//test//
			// get the four point depth//
			/////////////////////////////////////////////////////////////////////////
			std::fstream out6("D:\\fourthdepth.txt", std::ios::app);
			for (int i = 0; i < MyContours.size(); i++) {
				out6 << MyContours[i].x << " " << MyContours[i].y << " "<<cv16img.at<USHORT>(MyContours[i].y, MyContours[i].x) << std::endl;
			}
			printflag = true;
			out6 << std::endl;
			//get the four point depth//
			//test //
			*/


			/*
		    //test//
			//用于测试限定区域内的深度值//
			std::fstream out10("D:\\testDepth.txt", std::ios::app);
			for (int i = 0; i < 424; i++) {
				for (int j = 0; j < 512; j++) {
					if (cv16img.at<USHORT>(i, j) > 0) {
						out10 << j << " " << i << " " << cv16img.at<USHORT>(i, j) << std::endl;
					}
				}
			}
			out10 << std::endl;
			out10 << std::endl;
			//用于测试限定区域内的深度值//
			//test//
			*/


//			GetHandPos(cv16img, MyContours, imgpt);    //dleete for contemporary retrieve the summitPoint of hand//





			//test//
			//测试对轮廓的深度调整//


			std::vector<cv::Point> vCopyPreContours;
			vCopyPreContours = MyContours;

			//test1:用于前后对比//
			std::vector<float>vPreDepth;
			float depth = 0;
			for (int i = 0; i < MyContours.size(); i++) {
				depth = cv16img.at<USHORT>(MyContours[i].y, MyContours[i].x);
				vPreDepth.push_back(depth);
			}
			//test:用于前后对比//

			depthRevise(cv16img, contours[i], MyContours);


			//test:测试深度值是否有改良//
			std::vector<float>vAfterDepth;
	
			for (int i = 0; i < MyContours.size(); i++) {
				depth = cv16img.at<USHORT>(MyContours[i].y, MyContours[i].x);
				vAfterDepth.push_back(depth);
			}
			
			//test:测试深度值是否有改良//

			//尖端点变化较大的点往小的方向变的点一定是手指尖端点//  //当平均值和给出的值差异较小时，说明这个尖端值是可靠的，可以使用，这时候看尖端值和平均值的关系，如果是变小的，这个变小的较大，一般为尖端点//

			//得到尖端的另外一种方法，这种要求手势几个边缘点之间最好有深度差，所以不适合在主持人在一边的情况//
			//test:测试这种确定尖端的方法的有效性//


			//test//
			//测试主持人在屏幕一侧的情形//
			float threshold = 15;
			cv::Point   fSummitPoint;



			bool bOneSideflag = getSummitPoint(threshold, MyContours, fSummitPoint);  //主持人站在屏幕的一边//  
			bool bInScreenFlag = true;

			

			//*利用深度调整算法获取到手势尖端点*//


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



				const int depthrevisethreshold = 80;
				//当该点的深度和平均值的深度差在阈值以内，我们认为改点的深度是可靠的，可以不需要对深度进行修正//

				//先不对深度的减小的变化量加以限制的情况下考虑//

				if (MinIndex > 0 && abs(MinDel) < depthrevisethreshold) {
					fSummitPoint = MyContours[MinIndex];
					bInScreenFlag = false;

					//可以测试一下，深度修正与否对最终的误差的影响//
					for (int i = 0; i < MyContours.size(); i++) {
						cv16img.at<USHORT>(MyContours[i].y, MyContours[i].x) = vCopyPreDepth[i];
					}

				}

				else {
					bInScreenFlag = true;
				}
				
				//*利用深度调整算法获取到手势尖端点*//
				

				//test//
				//利用K-Means方法获取到手势尖端点//
				if (bInScreenFlag && (MyContours.size()>0)) {
					//test//
					//用于获取到指尖的位置//

					fSummitPoint = getSummitPoint(contours[i], cv16img, MyContours);    //用于获取尖端的像素//
					//用于获取到指尖的位置//
				}


				//利用K-Means方法获取到手势尖端点//
				//test//
			}

			/*
			//test//
			//用于确定深度修正得瑟效果是否好，还是变的更坏了//
			//测试结果：测试了一组手的深度存在变化相对较大的情况，深度变好了//

			//用于获取未修正前的尖端点的深度//
			int PreSummitPointIndex = 0;
			for (int i = 0; i < vCopyPreContours.size(); i++) {
				if (fSummitPoint == vCopyPreContours[i]);
				PreSummitPointIndex = i;
			}
			srcSummitPosDepth = vPreDepth[PreSummitPointIndex];
			//用于获取未修正前的尖端点的深度//



			//用于获取未修正前的尖端点的深度//
			reviseSummitPosDepth = cv16img.at<USHORT>(fSummitPoint.y, fSummitPoint.x);
			//用于获取未修正前的尖端点的深度//

			//用于确定深度修正得瑟效果是否好，还是变的更坏了//
			//test//
			*/


			identPixelx = fSummitPoint.x;
			identPixely = fSummitPoint.y;




			//ps:对于状态丢失的点的状态可以用前面的状态替代//

			//test:测试这种确定尖端的方法的有效性//
			//得到尖端的另外一种方法，这种要求手势几个边缘点之间最好有深度差，所以不适合在主持人在一边的情况//


			

			//测试对轮廓的深度调整//
			//test//




			//*********************problem***************************//

			//Kinect检测的深度数据还没有测，存在的问题是，在某种情况下，检测到的轮廓是在变化的，因此存在识别的边缘不准确，这个需要测试一下//

			//*********************problem***************************//




			/*
			std::vector<Point3f> vdistBetween;
			float fdistbetween = 0;

			for (int i = 0; i < MyContours.size()-1; i++) {
				for (int j = i + 1; j < MyContours.size(); j++) {
					fdistbetween = abs(cv16img.at<USHORT>(MyContours[i].y, MyContours[i].x) - cv16img.at<USHORT>(MyContours[j].y, MyContours[j].x));
					vdistBetween.push_back(Point3f(fdistbetween,i,j));
				}
			}
			*/



			/*
			cv::Point3f fMinDepth(10000,0,0);
			for (int i = 0; i < vdistBetween.size(); i++){
				Point3f p = vdistBetween[i];
				if (fMinDepth.x > p.x) {
					fMinDepth = p;
				}
			}

			Point2i p((int)(fMinDepth.y), (int)(fMinDepth.x));
			std::vector<Point> vCopyMyContours;
			for (int i = 0; i < MyContours.size(); i++) {
				if (i != p.x  && i != p.y) {
					vCopyMyContours.push_back(MyContours[i]);
				}
			}


			*/



			/*

			UINT testDepth = 0;
			int m = 0;
			for (int i = 0; i < MyContours.size(); i++) {
				if (cv16img.at<USHORT>(MyContours[i].y, MyContours[i].x) > testDepth) {
					testDepth = cv16img.at<USHORT>(MyContours[i].y, MyContours[i].x);
					m = i;
                }
			}  */

//			cen = MyContours[m];


//			cen = pMyPoint;

			//利用轮廓最小外接圆圆心和轮廓的边界求值，确定手势的边界点//
			//test//


			/*

			imgpt.x = cen.x;
			imgpt.y = cen.y;
			imgpt.z = cv16img.at<USHORT>(cen.y, cen.x);

			*/


			//test :利用指间点必定是边界点得到指间点的位置//

			//test :利用指间点必定是边界点得到指间点的位置//





			identPixel = MyContours;


			/*
			//test//
			//获取轮廓像素点的值//
			for (int k = 0; k < contours[i].size(); k++) {
				out1 << contours[i][k].x << "  " << contours[i][k].y << " " << cv16img.at<USHORT>(contours[i][k].y, contours[i][k].x) << std::endl;
			}
			out1 << std::endl;
			out1 << std::endl;
			out1 << std::endl;
			out1 << std::endl;
			//获取轮廓像素点的值//
			//test//
			*/


			imgpt.x = fSummitPoint.x;
			imgpt.y = fSummitPoint.y;
			imgpt.z = cv16img.at<USHORT>(imgpt.y, imgpt.x);



			V3f vImgpt;
			vImgpt.x = imgpt.x;
			vImgpt.y = imgpt.y;
			vImgpt.z = imgpt.z;
	

			

			//test//
			//用于判断手势是否在识别区域内//
			//因为限制区域并不是我们的识别区域，因此在此地还需要判断给出的店是否在识别区域，进行判断，存在这种情况，该点可以被识别到，但是该点不在识别区域中，因此需要对点是否在识别区域进行判断//

			V3f KinectPos = getKinectPos(vImgpt);

			//获取手势点的Kinectpos,为确定手势点的前两帧服务//
			vfKinectPos.push_back(KinectPos);  //有多少只手，vfKinectPos的size就有多大//



			bool bIsInFlag = IsInZone(KinectPos);  //用于判断点是否在识别区域内，从而得到该点的状态//
			//当点在识别区域中时，用于判断该点离边界点最近的距离dist或者是次近的点,同时该点的速度方向与该平面的方向一致，且小于阈值，则认为下一刻若点识别不到，则认为是离开了屏幕//

			//在此先假设已经得到了手的速度方向，该速度方向必须为当前帧和前一帧的KinectPos//

			const float fOutThreshold = 10;  //用于判断下一个点是否要出识别区域的阈值，如果下一个点无法识别到//
			std::vector<cv::Point3f> vLastTwoPos;

			vLastTwoPos.resize(2);  //vLatTwoPos的值必须通过该手势上一个pos和当前pos共同得到//  //*************必须调用手势匹配获取到该手势的上一个pos,有对应的判断函数//
			//获取上一个手势的pos的判断函数//

			int index = 0;  //用于得到上一帧对应的handPos点//
			cv::Point3f p = cv::Point3f(KinectPos.x, KinectPos.y, KinectPos.z);

			//若为true:表示当前帧与前一帧某一手势存在匹配; false不存在匹配，该手势为新手势,那么将其前一帧和当前帧的位置设为相同，是为了表示当前帧的速度0,即用当前点替代下一个点，如果下一点找不到//
			if (getMarchLastHandPos(vCopyPreHands, p, fHandPosThreshold, index))   
			{
				vLastTwoPos[0] = cv::Point3f(vCopyPreHands[index].worldPos.x, vCopyPreHands[index].worldPos.y, vCopyPreHands[index].worldPos.z);
				vLastTwoPos[1] = cv::Point3f(KinectPos.x, KinectPos.y, KinectPos.z);
			}

			else {
				vLastTwoPos[0] = cv::Point3f(KinectPos.x, KinectPos.y, KinectPos.z);
				vLastTwoPos[1] = cv::Point3f(KinectPos.x, KinectPos.y, KinectPos.z);
			}

			cv::Point3f pSpeed = vLastTwoPos[1] - vLastTwoPos[0];
			float fSpeedLength = sqrt(pSpeed.x * pSpeed.x + pSpeed.y * pSpeed.y + pSpeed.z * pSpeed.z);

			//表示有两帧的数据//
	
				V3f vfPospre, vfPosAfter;
				vfPospre.x = vLastTwoPos[0].x; vfPospre.y = vLastTwoPos[0].y;  vfPospre.z = vLastTwoPos[0].z;
				vfPosAfter.x = vLastTwoPos[1].x; vfPosAfter.y = vLastTwoPos[1].y;  vfPosAfter.z = vLastTwoPos[1].z;
				//计算坐标点到五个平面的距离；计算速度点到五个平面的距离：利用前后点到平面的距离变化如果是变小了，则说明速度是朝这个方向//
				std::vector<int> vDir; //当速度朝向该方向时候，对应的数组内的值设为1，否则设为0//
				std::vector<float> fPosDist;
				fPosDist.resize(5);
				std::vector<float> vPreposDist;
				std::vector<float> vAfterposDist;
				vPreposDist.resize(5);
				vAfterposDist.resize(5);


				//给出检测到的手势  但是此时的m_hands是对应的每个手势是没有ID的//
				sAction act;
				//给出检测到的手势  但是此时的m_hands是对应的每个手势是没有ID的//

				if (bIsInFlag) {

					ConvertImagePointToHand(vImgpt, act, vCopyPreHands);  //get kinect coordnitate pos//

					//对五个平面进行细分//
					Plane3f p1, p2, p3, p4, p5;

					// vKinectPos标记的四个点；vdistMarkPoint:离屏的四个点 
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

					//如果下一个点无法识别到，判断该点是否是因为出了识别区域引起的//

					
					for (int i = 0; i < vDir.size(); i++) {
						if (vDir[i] == 1) {
							if (fPosDist[i] < fOutThreshold) {
								bIsInOrOutFlag = true;
								break;
							}

						}
					}
					bIsInOrOutFlag = false;
					//如果下一个点无法识别到，判断该点是否是因为出了识别区域引起的//

				}

				//如果该点未在识别区域中// 
				else
				{
					act.st = Up;
				}

				m_hands.push_back(act);

			//用于判断手势是否在识别区域内//
			//test//

		}

		//*************上述都在获取尖端点里面*****************//

	}
	//上述是对手势轮廓进行检测//


	//************************** need to consider further *************************//
	//获取前两帧的手势的KinectPos//  //有一点需要考虑的是m_hands.size() 这一部分//        //考虑vPos怎么获取到第一个数据的：如果vPos的size为0则利用当前的m_Hands获取第一组数据，如果vPos的size不为0，则利用前一帧和当前帧进行组合获取vPos的数据//


	//当为非初始帧时//
	if (vPos.size() > 0) {
		float fDist = 0;
		float fMinDist = 1000;
		int iMinIndex = 0;
		cv::Point3f pKinectPos;


		//先利用固定的阈值进行判断，后续再利用存储的hand点的speed来对阈值进行更新//
		vHandPosThreshold.resize(vCopyPreHands.size());
		for (int i = 0; i < vCopyPreHands.size(); i++) {
			vHandPosThreshold[i] = fHandPosThreshold;
		}
		//先利用固定的阈值进行判断，后续再利用存储的hand点的speed来对阈值进行更新//


		//上一帧和当前帧的pos进行匹配的方法： 首先利用前一帧的手势和当前帧的手势进行比较，获取到当前帧与前一帧手势的点的对应关系，从而可以vidPospush_back(ID/pos),然后对于后面更新的m_Hands将vpos赋予，当前一帧没有手势数据时//
		//可以利用当前的m_Hands自动赋予//

		//***********************如果同时出现手势丢失和有新的手势进来，怎么处理**************************//
		//处理方法：手势丢失或者手势离开的同时有新手势进来：利用阈值进行判定，如果上一帧判定的手势的在屏幕坐标和当前帧的坐标距离差超过阈值的2倍，则认为不是同一个手势，否则认为vCopyPreHands[k]为丢失掉的手势，m_Hands为新增的手势//

		//*************假设已经获取到前一帧手势的阈值:vHandPosThreshold，这是下一步要处理的***************//
		//vHandPosThreshold:有一个默认值，正常情况下的两点之间的距离，同时针对不同的hand运动情况赋予不同的值，现在只考虑常规速度情况，在初始化中设定，后续可以改进//


		//1:前一帧的手势比当前帧的手势要少//
		//解释:是以前一帧的m_Hands为参照物，因此ID是按照前一帧顺下来，所以vPos的对应关系中不需要ID//


		//为了不至于引起混乱，可以先将当前帧的pos调整顺序，最后统一赋予vPos//

		std::vector<cv::Point3f> vfPos;  //用于记录与前一帧ID匹配的pos//
		std::vector<int> vMatchIndex;


		if (vCopyPreHands.size() < m_hands.size()) {
			for (int i = 0; i < vCopyPreHands.size(); i++) {
				for (int j = 0; j < m_hands.size(); j++) {

					//已经匹配好的点就不用再匹配了//
					bool bIsMatch = false;
					for (int k = 0; k < vMatchIndex.size(); k++) {
						if (vMatchIndex[k] == j) {
							bIsMatch = true;
							break;
						}
					}
					//已经匹配好的点就不用再匹配了//
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

				//对于大于阈值的2倍情形：前一帧为丢失或者出去的handPos，当前帧对应的hand为新增的handPos,其中需要和对应的Hand的距离阈值进行匹配//
				if (fMinDist > 2 * vHandPosThreshold[i]) {
					vfPos.push_back(cv::Point3f(i, -1, -1));  //表示对应的前一帧为丢失的关键帧，-1，-1为标志位//
				}

				else {
					vMatchIndex.push_back(iMinIndex);
					vfPos.push_back(cv::Point3f(vfKinectPos[iMinIndex].x, vfKinectPos[iMinIndex].y, vfKinectPos[iMinIndex].z));
				}

				fMinDist = 10000;
				iMinIndex = 0;
			}
			//对于j中未分配的pos直接加到vfPos的后面//
			
			//利用的思想为：在vMathcIndex中没有的Index则是新多出来的手势，是要被重新分配的//
			for (int k = 0; k < m_hands.size(); k++) {
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
			vMatchIndex.clear();   //任何一个vector用完之后一定要clear，防止对其他的程序带来影响//
		}


		



		//2:前一帧的手势比当前帧的手势多//

		else {
			//步骤1：匹配，找到对应关系//


			//对于当前帧匹配上但是与前一帧的距离大于阈值的手势pos，记录下来赋予新的ID//
			std::vector<cv::Point3f> vCurrentNewHand;



			for (int i = 0; i < m_hands.size(); i++) {
				for (int j = 0; j < vCopyPreHands.size(); j++) {

					//已经匹配好的点就不用再匹配了//
					bool bIsMatch = false;
					for (int k = 0; k < vMatchIndex.size(); k++) {
						if (vMatchIndex[k] == j) {
							bIsMatch = true;
							break;
						}
					}
					//已经匹配好的点就不用再匹配了//

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

				//对于大于阈值的2倍情形：前一帧为丢失或者出去的handPos，当前帧对应的hand为新增的handPos,其中需要和对应的Hand的距离阈值进行匹配//
				if (fMinDist > 2 * vHandPosThreshold[iMinIndex]) {
					vfPos.push_back(cv::Point3f(iMinIndex, -1, -1));  //表示对应的前一帧为丢失的关键帧，-1，-1为标志位//
					vCurrentNewHand.push_back(pKinectPos);                      //记录下此时的m_Hands的pos作为新的手势//
				}


				else {
					vMatchIndex.push_back(iMinIndex);
					vfPos.push_back(cv::Point3f(vfKinectPos[iMinIndex].x, vfKinectPos[iMinIndex].y, vfKinectPos[iMinIndex].z));
				}

				fMinDist = 10000;
				iMinIndex = 0;
			}

			//对前一帧未加入进来的hand的pos赋值为cv::Point3f(0,0,0)//

			//利用的思想为：在vMathcIndex中没有的的Index则是当前一帧的少的手势，需要在接下来进行处理//
			
			//同时对前一帧有当前帧没有的hands,需要记录下对应的ID，供下面对该hand进行处理//

			for (int k = 0; k < vCopyPreHands.size(); k++) {
				int l = 0;
				bool flag = true;
				while (l < vMatchIndex.size() && flag) {
					if (k == vMatchIndex[l]) {
						flag = false;
					}
					l++;
				}
				if (flag) {
					vfPos.push_back(cv::Point3f(k,-1,-1));  //Point3f(k,-1,-1)：第一位是为了表示当前帧丢失掉的或者是出去的hand的ID,后面的-1，-1作为标志位，与其他的KinectPos区别开来//
				}
			}


			//将当前帧与前一帧匹配但是大于阈值的hand的pos赋予vfPos//
			for (int m = 0; m < vCurrentNewHand.size(); m++) {
				vfPos.push_back(vCurrentNewHand[m]);
			}
			//将当前帧与前一帧匹配但是大于阈值的hand的pos赋予vfPos//


			vMatchIndex.clear();   //任何一个vector用完之后一定要clear，防止对其他的程序带来影响//
		}

		//利用得到的vfPos对vPos进行更新//
		for (int i = 0; i < vfPos.size(); i++) {
			if (i < vPos.size()) {
				if (vPos.size() < 2) {
					vPos[i][vPos.size()] = vfPos[i];
				}

				else {
					vPos[i][vPos.size() - 1] = vPos[i][vPos.size()];
					vPos[i][vPos.size()] = vfPos[i];
				}
			}

			else {
				std::vector<cv::Point3f> VNewHands;
				VNewHands.push_back(vfPos[i]);
				vPos.push_back(VNewHands);
			}

		}

		//利用得到的vfPos对vPos进行更新//
		




//delete for contemporary//

		//vCopyPreHands与vPos是对应的，都是上一帧的遗留下的情形//    //*******待会还要考虑没有上一帧的情形*********//
/*		for (int i = 0; i < vCopyPreHands.size(); i++) {

			//需要判断该点就是该ID对应的手势//
			//利用pos的距离最小来判断  如果vPos的size为0说明之前是没有手势的，则直接将手势存储下来即可，这个需要单独讨论，必须利用当前的m_Hands来讨论 ，先讨论前面的利用前面的手势进行讨论  //
			//还有一种情况，当前的手势比前一个手势多了怎么处理？//  //a problem need to be worked out//  //先考虑手势减少的或者与上一帧手势一致的情形//

			//考虑上一帧存在手势的情形//   

			

			pKinectPos = cv::Point3f(vKinectPos.x, vKinectPos.y, vKinectPos.z);
			cv::Point3f distp = pKinectPos - vPos[i][vPos[i].size() - 1];
			fDist = sqrt(distp.x * distp.x + distp.y * distp.y + distp.z * distp.z);
			if (fMinDist > fDist) {
				fMinDist = fDist;
				iMinIndex = i;
			}

			vPos[i].push_back(Point3f(vKinectPos.x, vKinectPos.y, vKinectPos.z));
		}

		if (vPos[iMinIndex].size() < 2) {
			vPos[iMinIndex].push_back(pKinectPos);
		}

		else {
			vPos[i][0] = vPos[i][1];
			vPos[i][1] = Point3f(vKinectPos.x, vKinectPos.y, vKinectPos.z);
		}

		*/
		//delete for contemporary//
	}


	//  //当为初始帧时//                   //以后写程序的时候如果if后面需要带else一定要先写上，防止在后面忘掉////  
	//初始帧对应的vPos点即为当前的hands的pos//
	else {
		vPos.resize(m_hands.size());
		for (int i = 0; i < vPos.size(); i++) {
			vPos[i].push_back(cv::Point3f(m_hands[i].worldPos.x, m_hands[i].worldPos.y, m_hands[i].worldPos.z));
		}
	}



	//在前一帧有手势的前提下//
	//建立手的对应关系//
	float fMinDist = 0;

	std::vector<sAction> vCurrentHands = m_hands;

	//获取上一个手势的ID，为下面对离开的手势和失去的手势确认服务//
	std::vector<int> vLastHandIDIndex;
	for (int i = 0; i < vCopyPreHands.size(); i++) {
		vLastHandIDIndex.push_back(vCopyPreHands[i].ID);
	}


	if (m_hands.size() < vCopyPreHands.size()) {
		for (int i = 0; i < m_hands.size(); i++) {
			int iMinindex = 0;                              //获取当前手势点对应的前一个手势点的指数//
			for (int j = 0; j < vCopyPreHands.size(); j++) {
				float fDist;
				cv::Point2f p;
				p.x = m_hands[i].pos.x - vCopyPreHands[j].pos.x;
				p.y = m_hands[i].pos.y - vCopyPreHands[j].pos.y;
				fDist = sqrt(p.x * p.x + p.y * p.y);
				if (fMinDist > fDist) {
					fMinDist = fDist;
					iMinindex = j;
				}
			}
			vCopyPreHands[i].ID = vCopyPreHands[iMinindex].ID;
		}


		m_hands = vCopyPreHands;


		//找到丢掉的那一个手势的ID//
		int iLostHandIndex = 0;
		int iLostOrOutID = 0;
		for (int i = 0; i < vLastHandIDIndex.size() ; i++) {
			bool bLostHandFlag = true;
			for (int j = 0; j < m_hands.size(); j++) {
				if (vLastHandIDIndex[i] == m_hands[j].ID) {
					bLostHandFlag = false;
					break;
				}
			}
			if (bLostHandFlag) {
				iLostHandIndex = i;
				break;
			}
		}

		iLostOrOutID = vLastHandIDIndex[iLostHandIndex];
		//找到丢失或者出去的那个手势的ID//




		//针对一个手势的情况//
		//如果标志位置flag = 0：表示手势为出去的手势，需要加一个出去的手势，如果标志位flag ==1 表示是未识别到，重置该点的位置//
		sAction sCurrentAction;
		bool flag;
		if (flag) {
			//未识别到该手势点，需要重置该手势//  //算法是利用计量帧的手势在屏幕上的位置变化，得到手势移动的速度和方向，考虑帧刷新的速度较快，在短时间内为匀速运动，得到当前帧的pos,利用前一帧的ACtion更新当前帧的Action//
			
			//程序记录下前两帧的对应手势的pos//   //在这里先设一下，在后面再添上// 
			std::vector<cv::Point> vPos;
			vPos.resize(2);
			cv::Point pSpeed = vPos[1] - vPos[0];
			cv::Point pCurrentPos = vPos[1] + pSpeed;
			sCurrentAction.ID = iLostOrOutID;
			sCurrentAction.pos.x = pCurrentPos.x;
			sCurrentAction.pos.y = pCurrentPos.y;
			sCurrentAction.st = vCopyPreHands[iLostHandIndex].st;
		}
		else {
			//手势出了识别区域，需要设置标志位//
			sCurrentAction.ID = iLostOrOutID;
			sCurrentAction.st = Up;
			sCurrentAction.pos.x = 0;
			sCurrentAction.pos.y = 0;
		}

		vCopyPreHands.push_back(sCurrentAction);
		m_hands = vCopyPreHands;
		//对当前的手势进行更新//

		//ps：建立的前提条件是确定了手势是丢失或者是出去的flag，这是要下一步确定的；同时还需要存储每个手势前两帧的pos状态，这也是下一步要做的。//



	}
	//建立手的对应关系//

	if (m_hands.size() == 0)
	return (int)contours.size();
}



//需要知道前一帧的手势来判断当前帧的手势状态,前提是该手势必须在识别区域中//
//首先找到当前手势与前一帧的匹配手势；//
//1：如果前一帧无匹配手势，那该手势为down；//
//2:如果匹配的前一帧手势为up，该手势为down;//
//3:如果前一帧手势为down,利用距离进行判断，如果距离大于阈值,则判断为Move否则为down;  //该距离最好设为屏幕距离//  //
//4:如果前一帧状态为Move,当前帧距离与前一帧距离大于阈值，则仍为Move，否则为none//
//5: 如果前一帧的手势为none，说明前一帧处于等待状态中，如果在屏幕上的移动距离大于Radius,则认为变成Move,否则仍为none//
void CDepthBasics::ConvertImagePointToHand(const V3f& img, sAction& act,const std::vector<sAction> preHands)
{
	int index = 0;
	V3f p = getKinectPos(img);
	V3f pScreenProjectp = getProjectPos(p);
	V3f pMid = convertTo(pScreenProjectp);

	cv::Point3f pKinectPos = cv::Point3f(p.x, p.y, p.z);
	bool  bHasMatchPoint = getMarchLastHandPos(preHands, pKinectPos, fHandPosThreshold, index);
	if (!bHasMatchPoint) {
		act.st = Down;
	}

	else {

		float distance = sqrt((pMid.x - preHands[index].pos.x) * (pMid.x - preHands[index].pos.x) + (pMid.y - preHands[index].pos.y) * (pMid.y - preHands[index].pos.y));

		if (preHands[index].st == Up) {
			act.st = Down;
		}

		else if (preHands[index].st == Down) {
			
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

		else {
			if (distance > Radius) {
				act.st = Move;
			}
			else {
				act.st = None;
			}
		}
	}

	//获取当前手势的屏幕坐标//
	act.pos.x = pMid.x;
	act.pos.y = pMid.y;
	act.pos.z = pMid.z;

	//获取当前手势的全局坐标//
	act.worldPos.x = p.x;
	act.worldPos.y = p.y;
	act.worldPos.z = p.z;
}



V3f CDepthBasics::getKinectPos(const V3f & pos) 
{

	V3f p;
	p.x = (pos.x - 256) *pos.z / foc;
	p.y = (pos.y - 212)  *pos.z / foc;
	p.z = pos.z;
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

//将屏幕的kinect三维坐标转换成屏幕的二维坐标//
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

	USHORT maxdepth = 10;

	cv::Mat cvimg = cv16img;
	for (int i = 0; i <  cDepthHeight; i++) {
		for (int j = 0; j < cDepthWidth; j++) {
			V3f pImg;
			pImg.x = j;
			pImg.y = i;
			pImg.z = cv16img.at<USHORT>(i, j);

			if (((pImg.z < iMaxDepth) && (pImg.z > iMinDepth)) && (pImg.x > minPixelx)  && (pImg.x < maxPixelx)  && (pImg.y > minPixely)  && (pImg.y < maxPixely)) {
				V3f p = getKinectPos(pImg);

				V3f projectP = getProjectPos(p);

				float fdist = (projectP - p).length();

				if (  fdist > 60) {

					maxdepth = cv16img.at<USHORT>(i, j);


					//test//
					//之前测试用识别区域对图像进行裁剪，但是处理速度较慢，因此注释掉//
					/*

					if (!IsInZone(p) || fdist < 50) {
					cv8img.at<uchar>(i, j) = 0;
					cvimg.at<USHORT>(i, j) = 0;
					}
					else {
					if (maxdepth < cv16img.at<USHORT>(i, j)) {
					maxdepth = cv16img.at<USHORT>(i, j);
					}
					}
					//之前测试用识别区域对图像进行裁剪，但是处理速度较慢，因此注释掉//
					//test//
					*/
				}

				else  {
					cv8img.at<uchar>(i, j) = 0;
					cvimg.at<USHORT>(i, j) = 0;
//					cv16img.at<USHORT>(i, j) = 0;
				}
			}

			else
			{
				cv8img.at<uchar>(i, j) = 0;
				cvimg.at<USHORT>(i, j) = 0;
//				cv16img.at<USHORT>(i, j) = 0;
			}

			//利用三维点去判断这个点是否在识别区域里//

			

			

		}
	}

	

	//for test//

	/*
	bool testflag = true;
	for (int i = 0; i < 424; i++) {
		for (int j = 0; j < 512; j++) {

			if (cv16img.at<USHORT>(i, j) > 0) {

				if ((markpixelx != 0 || markpixely != 0) && testflag) {
					setPixel(j, i);
					testflag = false;
				}
				
			}
		}
	}
	*/
	//for test//

	
	/*
	//for test//
	//获取整幅图像的深度信息//
	std::fstream out4("D:\\depthinfo.txt", std::ios::app);
	for (int i = 0; i < 424; i++) {
		for (int j = 0; j < 512; j++) {
			if (cv16img.at<USHORT>(i, j) > 0) {
				out4 << j << " " << i << " " << (float)(cv16img.at<USHORT>(i, j)) << std::endl;
			}
		}
	}
	std::cout << std::endl;
	//获取整幅图像的深度信息//
	//for test//
	*/
	

	for (int i = 0; i < cDepthHeight; i++) {
		for (int j = 0; j < cDepthWidth; j++) {
			cv8img.at<uchar>(i, j) = (uchar)(cvimg.at<USHORT>(i, j) * 255.0 / maxdepth);
		}
	}
	return true;
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



Plane3f CDepthBasics::GetPlane(V3f  & v1, V3f & v2, V3f & v3, V3f & v4)
{
	V3f vScreen1, vScreen2, vScreen3, vScreen4;
	vScreen1 = v1;
	vScreen2 = v2;
	vScreen3 = v3;
	vScreen4 = v4;
	Plane3f screenPlane(v1, v2, v3);

	V3f normalVector = screenPlane.normal;
	v1 = v1 + h * normalVector;
	v2 = v2 + h * normalVector;
	v3 = v3 + h * normalVector;
	v4 = v4 + h * normalVector;
	Plane3f screenPlane1(v1, v2, v3);
	Line3f line1(V3f(0, 0, 0), vScreen1);
	Line3f line2(V3f(0, 0, 0), vScreen2);
	Line3f line3(V3f(0, 0, 0), vScreen3);
	Line3f line4(V3f(0, 0, 0), vScreen4);
	screenPlane1.intersect(line1, v1);
	screenPlane1.intersect(line2, v2);
	screenPlane1.intersect(line3, v3);
	screenPlane1.intersect(line4, v4);
	return screenPlane1;
}



bool CDepthBasics::IsInZone(const V3f  & pKinectPos)
{



	V3f vInterSect1, vInterSect2;

	Line3f line(V3f(0, 0, 0), pKinectPos);
	pScreenPlane.intersect(line, vInterSect1);
	pdisScreenPlane.intersect(line, vInterSect2);

	

	if (IsBetweenTwoPlane(vInterSect1, pKinectPos, vInterSect2))
	{
		if (GetPositionLocation(vKinectPos[0], vKinectPos[1], vKinectPos[2], vKinectPos[3], vInterSect1))
		{
			if (GetPositionLocation(vdisMarkPoint[0], vdisMarkPoint[1], vdisMarkPoint[2], vdisMarkPoint[3], vInterSect2))
				return true;
		}
	}
	return false;
}



//获取到手势坐标在屏幕平面的投影点，该投影点坐标为Kinect坐标系下的坐标//
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
	UINT minDepth = max(max(vdisMarkPoint[0].z, vdisMarkPoint[1].z), max(vdisMarkPoint[2].z, vdisMarkPoint[3].z));
	return minDepth;

}



void  CDepthBasics::setPixel(int markpixelx ,int markpixely) {
	this->markpixelx = markpixelx;
	this->markpixely = markpixely;
}



void CDepthBasics::on_trackbar(int pos, std::vector<vector<Point>> & contours, cv::Mat & cv16Img)
{
	// 转为二值图
	cvThreshold(g_pGrayImage, g_pBinaryImage, pos, 255, CV_THRESH_BINARY);
	// 显示二值图

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


//对深度值进行修正//
void CDepthBasics::depthRevise(cv::Mat & cv16Img, std::vector<Point> & srcContours,std::vector<Point> & contours) 
{
	//test//
	//修正轮廓//

	std::vector<int> vdelPointIndex;//需要被删除的点的序号//  
	
	
	for (int i = 0; i < contours.size(); i++)
	{


		//在此间p总是在变化的,需要得到不变的点的深度值//
		cv::Point2f srcPoint = contours[i];

		//获取替代的像素点//
		
		cv::Point2f pCenter;   //存储一个中心标志点//

		std::vector<cv::Point2f> pNeighbour;
		cv::Point2f p = contours[i];
		getValidInnerPoint(cv16Img, srcContours, p);
		//获取替代的像素点//

		//邻近方向//
		pNeighbour = getNeighbour(p);
		//邻近方向//

		//获取该像素点的有效深度值//


		/*
		//test:观察深度值是否有变换//
		float preDepth = cv16Img.at<USHORT>(p.y, p.x);
		//test:观察深度值是否有变换//
		*/

		bool  bIsavgDepthValid = avgDepth(p, srcContours, cv16Img);


		/*
		float afterDepth1 = cv16Img.at<USHORT>(p.y, p.x);
		*/


		if (bIsavgDepthValid) {
			cv16Img.at<USHORT>(srcPoint.y, srcPoint.x) = cv16Img.at<USHORT>(p.y, p.x);
		}

		//test:观察深度值是否有变换//


		std::vector<int>vNeighbour;
		vNeighbour.resize(100);
		int iLayer = 0;

		pCenter = p;


		//规定一个标志位，当达到该标志位时，while循环退出，该轮廓点也被设定为无效点//
		int Mark = 0;   //当该标志位到达1,表示进入第三层循环时自动退出循环//

		while (!bIsavgDepthValid && Mark ==0) {
			p = (pNeighbour[vNeighbour[iLayer]]);
			vNeighbour[iLayer] = vNeighbour[iLayer] + 1;

			//如果该点的四周都没有好点//
			if (vNeighbour[iLayer] == 8) {
				if (iLayer > 0 ) {

					if (vNeighbour[iLayer - 1]==7) {
						std::cout << "hehe" << std::endl;
					}

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
			//如果该点的四周都没有好点//
			if (Mark == 0) {
				if (cv16Img.at<USHORT>(p.y, p.x) > 0) {
					getValidInnerPoint(cv16Img, srcContours, p);
					bIsavgDepthValid = avgDepth(p, srcContours, cv16Img);
				}

			}
		}
		//获取该像素点的有效深度值//

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
		//对深度值进行测试，看是否有坏点//

	    /*
		std::fstream out20("D:\\checkDepth.txt", std::ios::app);
		for (int i = 0; i < vOridinal.size(); i++) {
			out20 << vOridinal[i] << std::endl;
		}
		out20 << std::endl;
		out20 << std::endl;
		*/


		//对深度值进行测试，看是否有坏点//
		//test//

	
	//对深度修真//

	//修正轮廓//
	//test//
}

//对于识别的轮廓，确定给定的点是否是边界点,同时要满足改点为内部点//
bool CDepthBasics::isBorderOrOut(const cv::Mat & cv16Img, const std::vector<Point> & contours, const cv::Point & p) {
	if (cv16Img.at<USHORT>(p.y, p.x) < 10)
		return true;
	for (int i = 0; i < contours.size(); i++) {
		if (p == contours[i])
			return true;
	}
	return false;
}



//通过给定的点，对该点的深度进行修正//

bool CDepthBasics::avgDepth(const cv::Point2f & p, const std::vector<Point> & srcContours, cv::Mat & cv16Img)
{
	USHORT depth1 = 0, depth2 = 0, depth3 = 0, depth4 = 0, depth5 = 0, depth6 = 0, depth7 = 0, depth8 = 0, depth9 = 0;
//	cv::Point2f p, pLeft, pRight, pUp, pDown, pLeftUP, pLeftDown, pRightUp, pRightDown;

	cv::Point2f  pLeft, pRight, pUp, pDown, pLeftUP, pLeftDown, pRightUp, pRightDown;


	float depth = 0;
	int Count = 1;

	std::vector<cv::Point3f> vOridinal;     //for test :在计算平均值的过程中是否存在坏点//

		
		pLeft = cv::Point2f(p.x - 1, p.y);
		pRight = cv::Point2f(p.x + 1, p.y);
		pUp = cv::Point2f(p.x, p.y - 1);
		pDown = cv::Point2f(p.x, p.y + 1);


		//对角线方向//
		pLeftUP = cv::Point2f(p.x - 1, p.y - 1);
		pLeftDown = cv::Point2f(p.x - 1, p.y + 1);
		pRightUp = cv::Point2f(p.x + 1, p.y - 1);
		pRightDown = cv::Point2f(p.x + 1, p.y + 1);
		//对角线方向//

		depth1 = cv16Img.at<USHORT>(p.y, p.x);
		depth = depth + depth1;
		vOridinal.push_back(Point3f(p.x, p.y, depth1));


		if (!isBorderOrOut(cv16Img, srcContours, pLeft)) {
			depth2 = cv16Img.at<USHORT>(pLeft.y, pLeft.x - 1);
			depth = depth + depth2;
			Count++;

			vOridinal.push_back(Point3f(pLeft.x, pLeft.y, depth2));    //for test :在计算平均值的过程中是否存在坏点//

		}

		if (!isBorderOrOut(cv16Img, srcContours, pRight)) {
			depth3 = cv16Img.at<USHORT>(pRight.y, pRight.x + 1);
			depth = depth + depth3;
			Count++;

			vOridinal.push_back(Point3f(pRight.x, pRight.y, depth3));  //for test :在计算平均值的过程中是否存在坏点//

		}
		if (!isBorderOrOut(cv16Img, srcContours, pUp)) {
			depth4 = cv16Img.at<USHORT>(pUp.y - 1, pUp.x);
			depth = depth + depth4;
			Count++;

			vOridinal.push_back(Point3f(pUp.x, pUp.y, depth4));  //for test :在计算平均值的过程中是否存在坏点//

		}
		if (!isBorderOrOut(cv16Img, srcContours, pDown)) {
			depth5 = cv16Img.at<USHORT>(pDown.y + 1, pDown.x);
			depth = depth + depth5;
			Count++;

			vOridinal.push_back(Point3f(pDown.x, pDown.y, depth5));  //for test :在计算平均值的过程中是否存在坏点//
		}


		//将平均值到8个方向，如果Count较少//
		if (Count < 3) {
			if (!isBorderOrOut(cv16Img, srcContours, pLeftUP)) {
				depth6 = cv16Img.at<USHORT>(pLeftUP.y - 1, pLeftUP.x - 1);
				depth = depth + depth6;
				Count++;

				vOridinal.push_back(Point3f(pLeftUP.x, pLeftUP.y, depth6));  //for test :在计算平均值的过程中是否存在坏点//

			}

			if (!isBorderOrOut(cv16Img, srcContours, pLeftDown)) {
				depth7 = cv16Img.at<USHORT>(pLeftDown.y + 1, pLeftDown.x - 1);
				depth = depth + depth7;
				Count++;
				vOridinal.push_back(Point3f(pLeftDown.x, pLeftDown.y, depth7)); //for test :在计算平均值的过程中是否存在坏点//
			}

			if (!isBorderOrOut(cv16Img, srcContours, pRightUp)) {
				depth8 = cv16Img.at<USHORT>(pRightUp.y - 1, pRightUp.x + 1);
				depth = depth + depth8;
				Count++;

				vOridinal.push_back(Point3f(pRightUp.x, pRightUp.y, depth8));  //for test :在计算平均值的过程中是否存在坏点//
			}

			if (!isBorderOrOut(cv16Img, srcContours, pRightDown)) {
				depth9 = cv16Img.at<USHORT>(pRightDown.y + 1, pRightDown.x + 1);
				depth = depth + depth9;
				Count++;

				vOridinal.push_back(Point3f(pRightDown.x, pRightDown.y, depth9));  //for test :在计算平均值的过程中是否存在坏点//
			}
		}
		//将平均值到8个方向，如果Count较少//


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




			//可以通过bool来判断是否需要继续对vOridinalDepth进行调整： 当bool 为false时,即最大最小值的差在阈值的允许范围内，我们认为这样的数据是好数据，不需要修正，可以用此求得平均值，否则继续对数据进行修正//
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


//剔掉坏数据的函数//

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




//考虑到边界点的深度值不稳定，将边界点的邻域中的内部点替换掉边界点//
void CDepthBasics::getValidInnerPoint(const cv::Mat & cv16Img, const std::vector<Point> & srcContours, cv::Point2f & p)
{
	cv::Point2f  pLeft, pRight, pUp, pDown, pLeftUP, pLeftDown, pRightUp, pRightDown;
	    
	    //邻近方向//
		pLeft = cv::Point2f(p.x - 1, p.y);
		pRight = cv::Point2f(p.x + 1, p.y);
		pUp = cv::Point2f(p.x, p.y - 1);
		pDown = cv::Point2f(p.x, p.y + 1);
		//邻近方向//

		//对角线方向//
		pLeftUP = cv::Point2f(p.x - 1, p.y - 1);
		pLeftDown = cv::Point2f(p.x - 1, p.y + 1);
		pRightUp = cv::Point2f(p.x + 1, p.y - 1);
		pRightDown = cv::Point2f(p.x + 1, p.y + 1);
		//对角线方向//


		std::vector<Point2f> pneighbour;
		pneighbour.push_back(pLeft);
		pneighbour.push_back(pRight);
		pneighbour.push_back(pUp);
		pneighbour.push_back(pDown);


		//增加四个领域中的点，为获取有效的内部点//
		pneighbour.push_back(pLeftUP);
		pneighbour.push_back(pLeftDown);
		pneighbour.push_back(pRightUp);
		pneighbour.push_back(pRightDown);
		//增加四个领域中的点，为获取有效的内部点//


		int m = 0;    //pneighbour点中的序数//

		//利用内部点替换掉边界点//
		for (m = 0; m < 4; m++) {
			cv::Point pTest = pneighbour[m];

			if (!isBorderOrOut(cv16Img, srcContours, pTest)) {
				p = pneighbour[m];
				break;
			}
		}

		/*
		//test//
		//是否是四个点要么是边界点，要么是外部点//
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


		//测试M的数值//
		std::fstream out19("D:\\auxiliaryfourneighbourGetM.txt", std::ios::app);
		out19 << m << std::endl;


		//是否是四个点要么是边界点，要么是外部点//

		//test result:存在四个邻近点要么在识别区域外，要么同样为边界点，因此可以考虑将邻域点扩充8个点进行搜索，避免边界点的存在//

		//test//
		*/

		assert(m < 8);  //满足的时候不会加显示error，不满足的时候会显示error//
	
}

std::vector<cv::Point2f> CDepthBasics::getNeighbour(const cv::Point2f & p) {
	cv::Point2f pLeft, pRight, pUp, pDown, pLeftUP,pLeftDown,pRightUp,pRightDown;
	std::vector<cv::Point2f> pNeighbour;
	//邻近方向//
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
	//邻近方向//
	return pNeighbour;
}


//在调用者个函数的前提是必须设定一个阈值，当离平最近的距离点在阈值范围内时候，认为主持人是站在一侧的，可以使用该函数//
//当主持人站在屏幕的一侧的时候，手势可能不会水平，此时利用深度信息不再可行，考虑利用像素点的信息，因为此时所有的像素点位于一侧，而离这一侧距离最远的点近似是手势的尖端点//
//Input：opencv获取的轮廓点//
//output:手势的指尖点//

bool CDepthBasics::getSummitPoint(const float  & threshold, const std::vector<cv::Point>  & srcContours, cv::Point  & fSummitPoint) {


	//for test: 设置标志位//
	enum eDir {
		left,
		right,
		up,
		down
	};

	eDir ehordir, evertdir, esummitdir;
	//for test: 设置标志位//

	int iMinPixelx, iMaxPixelx, iMinPixely, iMaxPixely;   //识别的轮廓x和y方向上的最大最小像素坐标点//
	int iMaxIndex, iMinIndex,iMaxIndexy,iMinIndexy;       //对应x,y方向上最大最小像素点的的指数//
	
	float fMinDistx = 0, fMaxDistx = 0, fMinDisty = 0, fMaxDisty = 0;

	float fHorMinDist, fVertMinDist;  //获取水平方向和垂直方向距离边界最小的dist//

	iMinPixelx = iMinPixely = 10000;
	iMaxPixelx = iMaxPixely = 0;
	for (int i = 0; i < srcContours.size(); i++) {
		if (iMinPixelx > srcContours[i].x) {
			iMinPixelx = srcContours[i].x;
			iMinIndex = i;
		}

		if (iMinPixely > srcContours[i].y) {
			iMinPixely = srcContours[i].y;
			iMinIndexy = i;
		}

		if (iMaxPixelx < srcContours[i].x) {
			iMaxPixelx = srcContours[i].x;
			iMaxIndex = i;
		}

		if (iMaxPixely < srcContours[i].y) {
			iMaxPixely = srcContours[i].y;
			iMaxIndexy = i;
		}
	}

	//获取在x像素方向上的最小最大像素点离x边界的距离//
	fMinDistx = abs(minPixelx - iMinPixelx);
	fMaxDistx = abs(maxPixelx - iMaxPixelx);

	//获取在y像素方向上的最小最大像素点在y方向边界的距离//
	fMinDisty = abs(minPixely - iMinPixely);
	fMaxDisty = abs(maxPixely - iMaxPixely);


	//与阈值进行判断，确定该函数是否可用，若可用，判断手势的边界条件//


	std::vector<float> vMinDist;
	vMinDist.push_back(fMinDistx);  
	vMinDist.push_back(fMaxDistx);
	vMinDist.push_back(fMinDisty);
	vMinDist.push_back(fMaxDisty);

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
	//与阈值进行判断，确定该函数是否可用，若可用，判断手势的边界条件//
	
	


	switch (esummitdir) {
		case left: {
			fSummitPoint = srcContours[iMaxIndex];
			break;
		}

		case right: {
			fSummitPoint = srcContours[iMinIndex];
			break;
		}

		case up: {
			fSummitPoint = srcContours[iMaxIndexy];
			break;
		}

		case down: {
			fSummitPoint = srcContours[iMinIndexy];
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




//1：首先进行分类，获取两类包，2：然后取这两类包的平均深度，若深度较大的为手臂包，深度较小的为指间包，3：然后计算指间包到手臂包的中心的距离，最长的为指间//
//对于获取轮廓只有3个的情形，只需要走步骤1，2，剃掉有两个点的包，剩下的为指间包//
//对于获取的轮廓只有两个的情形，取取离屏较近的点//
//对于获取的轮廓只有一个的情形，直接使用//
cv::Point2f CDepthBasics::getSummitPoint(const std::vector<cv::Point> & srcContours, const cv::Mat & cv16Img, std::vector<cv::Point> & contours) {
	int k = 2;
	float threshold = 10;
	std::vector<std::vector<cv::Point>> vGroups;


	if (contours.size() == 1) {
		return contours[0];
	}

	else if (contours.size() == 2) {
		float depth1 = cv16Img.at<USHORT>(contours[0].y, contours[0].x);
		float depth2 = cv16Img.at<USHORT>(contours[1].y, contours[1].x);
		if (depth1 > depth2) {
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
			std::vector<float> vAvgDepth;
			float depth = 0;
			for (int i = 0; i < vGroups.size(); i++) {
				depth = getAvgDepth(cv16Img, vGroups[i]);
				vAvgDepth.push_back(depth);
			}

			//利用深度大小，是在当前测试环境下，在安装环境下，需要利用y或者z的信息//
			//现在是两个包的情形，选择depth最大的包作为剔除包，深度最小的包作为保存包，以深度较大的包为标准，在深度最小的包中，得到手势尖端点//
			float fMinDepth = 10000, fMaxDepth = 0;
			int iMinIndex = 0, iMaxIndex = 0;
			for (int i = 0; i < vAvgDepth.size(); i++) {
				if (fMinDepth >vAvgDepth[i]) {
					fMinDepth = vAvgDepth[i];
					iMinIndex = i;
				}
				if (fMaxDepth < vAvgDepth[i]) {
					fMaxDepth = vAvgDepth[i];
					iMaxIndex = i;
				}
			}
			//获取最大包的中心，以最小包点到该中心的距离点做为判断尖端点的标准//  //最大包是深度最小的点，最小包是深度最大的点//


			cv::Point2f referp = getOptimalPoint(vGroups[iMinIndex]);
			float fMinDist = 10000;
			int iMinDistIndex = 0;
			float fDist = 0;

			//为了增加算法的普适性,考虑以后k>2的情形，特别的当最小包的点数目为1时候特别的拎出来讨论一下//
			if (vGroups[iMaxIndex].size() == 1) {
				return vGroups[iMaxIndex][0];
			}

			else {
				for (int i = 0; i < vGroups[iMaxIndex].size(); i++) {
					cv::Point2f p(referp.x - vGroups[iMaxIndex][i].x, referp.y - vGroups[iMaxIndex][i].y);
					fDist = sqrt(p.x * p.x + p.y *p.y);
					if (fMinDist > fDist) {
						fMinDist = fDist;
						iMinDistIndex = i;
					}
				}
			}


			return vGroups[iMaxIndex][iMinDistIndex];
		}
	}

}


std::vector<std::vector<cv::Point>>  CDepthBasics::getGroup(const int & k, const std::vector<cv::Point>  & srcContours, const float  & threshold) {
	std::vector<std::vector<cv::Point>> vGroups;
	vGroups.resize(k);
	std::vector<cv::Point> vMarkPoint;
	vMarkPoint.resize(k);
	//设置初始的center点//
	for (int i = 0; i < k; i++) {
		vMarkPoint[i] = srcContours[i];
	}
	//设置初始的center点//

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
//求srcContours点的归属//

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

//求srcContours点的归属//
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
//获取到所有点的距离和最小的点//
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
//获取到所有点的距离和最小的点//
//belong to K-Means Algorithm//


//belong to getSummitPoint//
//获取在一个包中的边缘点的平均深度//
float CDepthBasics::getAvgDepth(const cv::Mat & cv16Img, std::vector<cv::Point> & contours) {
	float depth = 0;
	for (int i = 0; i < contours.size(); i++) {
		depth = depth + cv16Img.at<USHORT>(contours[i].y, contours[i].x);
	}
	depth = depth / (contours.size());
	return depth;
}
//获取在一个包中的边缘点的平均深度//
//belong to getSummitPoint//



//计算点到平面的距离//
void CDepthBasics::CalDistFromPointToPlane(const V3f &p, const Plane3f & plane, float dist) 
{
	V3f intersetPos;
	V3f dir = plane.normal;
	Line3f line;
	line.dir = dir;
	line.pos = p;
	plane.intersect(line, intersetPos);
	dist = (p - intersetPos).length;
}


//用于和前一帧的手势进行匹配//
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

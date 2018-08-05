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

//test//
float depth = 0;
int pixelx = 0;
int pixely = 0;



SHORT max_depth_regin[25650];
SHORT min_depth_regin[25];

float depthPixelx = 0;
float depthPixely = 0;

float  max_depth_cursor = 0;
float  min_depth_cursor = 10000;

int startTime = 0;
int  useTime = 0;
int endTime =  0;



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

}


//for test//
CDepthBasics::CDepthBasics(const std::vector<V3f> & vmarkscreenpos, const std::vector<V3f> & vmarkKinectPos, int KinectInstallPattern,float h, float h1, float Radius) :
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


	depthData.resize(218001);
	cv16Img = cv::Mat(cDepthHeight, cDepthWidth, CV_16UC1);
	for (int i = 0; i < cDepthHeight; i++)
	{
		for (int j = 0; j < cDepthWidth; j++) {
			cv16Img.at<USHORT>(i, j) = 0;
		}
	}
	m_PIHandTrace = HandTrace::createPIHandTrace();
	m_PIHandTrace->InitData(vmarkscreenpos, vmarkKinectPos, KinectInstallPattern, h, h1, Radius);
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

		startTime = clock();
		Update();

		int mode = 0;
		if (mode == 0) {
			int k = 0;
			if (depthData.back()== 1) {


				int t_ms = (FrameTime - m_nStartTime) / 1000;

				for (int i = 0; i < cDepthHeight; i++) {
					for (int j = 0; j < cDepthWidth; j++) {
							cv16Img.ptr<USHORT>(i)[j] = depthData[k];
						k++;
					}
				}
				k = 0;

				depth = cv16Img.ptr<USHORT>(pixely)[pixelx];

				m_PIHandTrace->SetVidient();

/*				startTime = clock();*/
				m_PIHandTrace->GetHandGesture(cv16Img);
				endTime = clock();
				useTime = endTime - startTime;

				depthData[218000] = 0;	
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

		//show SummitPoint Depth//
		USHORT TestSummitPointDepth = 1;
		USHORT TestSummitPointDepth1 = 1;
		int handNum = 0;
		std::vector<cv::Point3f> ContourSummitPoint1 = m_PIHandTrace->GetCounterSummitPoint3();
		if (ContourSummitPoint1.size() > 0 ) {
			handNum = ContourSummitPoint1.size();
			pixelx = ContourSummitPoint1.back().x;
			pixely = ContourSummitPoint1.back().y;
			int TestSummitPointPos = ContourSummitPoint1.back().y * nWidth + ContourSummitPoint1.back().x;
			TestSummitPointDepth = *(pBuffer + TestSummitPointPos);
			TestSummitPointDepth1 = ContourSummitPoint1.back().z;
		}

		//show SummitPoint Depth//
		std::vector<int>vTestTime = m_PIHandTrace->TestGetUseTime();
		int useTime1 = vTestTime[0];
		int useTime2 = vTestTime[1];

		float depth1 = cv16Img.at<USHORT>((int)(depthPixely), (int)(depthPixelx));
		//test//
		handNum = m_PIHandTrace->GetHandNum();

		StringCchPrintf(szStatusMessage, _countof(szStatusMessage), L" FPS = %0.2f    TestSummitPointDepth = %d  handNum=%d  TestSummitPointDepth1 =%d    depth =%d  depth1=%d\n", fps, TestSummitPointDepth, handNum, TestSummitPointDepth1, depth, depth1);

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

		std::vector<cv::Point2f> ContourSummitPoint =  m_PIHandTrace->GetCounterSummitPoint();
		std::vector<float> vTestPos(ContourSummitPoint.size());

		for (int i = 0; i < vTestPos.size(); i++) {
			vTestPos[i] = ContourSummitPoint[i].x + ContourSummitPoint[i].y * nWidth;
		}

		int Count = 0;
		while (pBuffer < pBufferEnd)
		{
			

			USHORT depth = *pBuffer;

			depthData[Count] = depth;
			Count++;


			// To convert to a byte, we're discarding the most-significant
			// rather than least-significant bits.
			// We're preserving detail, although the intensity will "wrap."
			// Values outside the reliable depth range are mapped to 0 (black).

			// Note: Using conditionals in this loop could degrade performance.
			// Consider using a lookup table instead when writing production code.
			BYTE intensity = static_cast<BYTE>((depth >= nMinDepth) && (depth <= nMaxDepth) ? (depth % 256) : 0);

			pRGBX->rgbRed = intensity;
			pRGBX->rgbGreen = intensity;
			pRGBX->rgbBlue = intensity;

			for (int i = 0; i < vTestPos.size(); i++)
			{
				if (abs(iCurse - vTestPos[i]) < 2)
				{
					pRGBX->rgbRed = 255;
					pRGBX->rgbGreen = 0;
					pRGBX->rgbBlue = 0;
				}
			}

			if (iCurse ==pos) {
				pRGBX->rgbRed = 255;
				pRGBX->rgbGreen = 0;
				pRGBX->rgbBlue = 0;
			}
			
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

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
#include "opencv/cv.h"
using namespace std;
using namespace cv;



SHORT max_depth_regin[25650];
SHORT min_depth_regin[25];

IplImage *g_pGrayImage = NULL;
IplImage *g_pBinaryImage = NULL;


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
//						out4 << i << " " << j <<" " <<cv16Img.at<USHORT>(i, j) << std::endl;
					k++;
				}
			}
			
			//for test check if the depth is right//

			k = 0;


			Precalc16_fast(cv16Img, cv8Img);
			SeekHands(cv16Img, cv8Img);

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



		StringCchPrintf(szStatusMessage, _countof(szStatusMessage), L" FPS = %0.2f    Time = %I64d", fps, (nTime - m_nStartTime));   //this time can be the timestamp//
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
		
		std::vector<int> vPos;
		for (int i = 0; i < identPixel.size(); i++) {
			int pos = (identPixel[i].y * nWidth + identPixel[i].x);
			vPos.push_back(pos);
		}

		
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

	
			

			
				pRGBX->rgbRed = intensity;
				pRGBX->rgbGreen = intensity;
				pRGBX->rgbBlue = intensity;


				
				//delete for contemporary//
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
	m_hands.clear();

	//test//
	//利用二值图获取轮廓//
	g_pGrayImage = &IplImage(cv8img);

	//将灰度图转化成二值图//
	g_pBinaryImage = cvCreateImage(cvGetSize(g_pGrayImage), IPL_DEPTH_8U, 1);
	std::vector<vector<Point>> contours;
	on_trackbar(200, contours, cv16img);
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

			//			CalcAveDepth(cv16img, contours[i]);


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
			const int threshold = 30;
			while (MyContours.size() < 5 && (oridinal.size()<threshold)) {
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


			//test//
			//对获取的轮廓点的深度进行修正//
			depthRevise(cv16img, contours[i], MyContours);
			//对获取的轮廓点的深度进行修正//
			//test//

			
			std::fstream out9("D:\\contours.txt", std::ios::app);
			for (int i = 0; i < contours.size(); i++) {
				for (int k = 0; k < contours[i].size(); k++) {
					out9 << contours[i][k].x << " " << contours[i][k].y << std::endl;
				}
			}

			out9 << std::endl;
			out9 << std::endl;
			


			//test : get the four point depth//
			/////////////////////////////////////////////////////////////////////////
			std::fstream out6("D:\\fourthdepth.txt", std::ios::app);
			for (int i = 0; i < MyContours.size(); i++) {
				out6 << MyContours[i].x << " " << MyContours[i].y << " "<<cv16img.at<USHORT>(MyContours[i].y, MyContours[i].x) << std::endl;
			}
			printflag = true;
			out6 << std::endl;
			//test : get the four point depth//



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




			GetHandPos(cv16img, MyContours, imgpt);



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


//			GetHandPos(cv16img, vCopyMyContours, imgpt);
//			GetHandPos(cv16img, contours[i], imgpt);

//			identPixelx = imgpt.x; 
//			identPixely = imgpt.y;

			identPixel = MyContours;

			/*

			identPixelx = MyContours[0].x;
			identPixely = MyContours[0].y;

			identPixelx1 = MyContours[1].x;
			identPixely1 = MyContours[1].y;

			identPixelx2 = MyContours[2].x;
			identPixely2 = MyContours[2].y;

			identPixelx3 = MyContours[3].x;
			identPixely3 = MyContours[3].y;
			*/

			for (int k = 0; k < contours[i].size(); k++) {
				out1 << contours[i][k].x << "  " << contours[i][k].y << " " << cv16img.at<USHORT>(contours[i][k].y, contours[i][k].x) << std::endl;
			}
			

			out1 << std::endl;
			out1 << std::endl;
			out1 << std::endl;
			out1 << std::endl;

			V3f vImgpt;
			vImgpt.x = imgpt.x;
			vImgpt.y = imgpt.y;
			vImgpt.z = imgpt.z;


			V3f vKinectPos = getKinectPos(vImgpt);

			bool flag = IsInZone(vKinectPos);



			/*
			if (!flag) {
				return -1;
			}
			*/

			/*
			sAction act;
			act.t = (FrameTime - m_nStartTime) / 1000;
			
			
			ConvertImagePointToHand(vImgpt, act);  //get kinect coordnitate pos//

			act.ID = m_hands.size();
			m_hands.push_back(act);

			out << act.ID << " " << act.pos.x << " " << act.pos.y << vImgpt << std::endl;
			*/

			/*
			if (m_hands.size() == MAX_HAND_NUM)
				return -1;
			*/

		}

	}






	if (m_hands.size() == 0)
	return (int)contours.size();
}








void CDepthBasics::ConvertImagePointToHand(const V3f& img, sAction& act)
{
	V3f p = getKinectPos(img);
	//for test//
	/*
	V3f img1 = img;
	img1.x = 262;
	img1.y = 153;
	img1.z = 1917;
	p = getKinectPos(img1);
	*/
	//for test//


    
	V3f pScreenProjectp = getProjectPos(p);

	V3f pMid;



	pMid = convertTo(pScreenProjectp);
	act.pos.x = pMid.x;
	act.pos.y = pMid.y;
	act.pos.z = pMid.z;
	act.st = None;




	float fdistBetweenHandScreen = (p - pScreenProjectp).length();
	 
	if (fdistBetweenHandScreen < h1) {
		act.st = Down;
		if (pCurrentPos.z != 0) {
			V3f pKinectPos = getKinectPos(pCurrentPos);
			V3f pScreenProjectp1 = getProjectPos(pKinectPos);
			V3f pMid1;
			pMid1 = convertTo(pScreenProjectp1);

			float distance = sqrt((pMid1.x - act.pos.x) * (pMid1.x - act.pos.x) + (pMid1.y - act.pos.y) * (pMid1.y - act.pos.y));
			if (distance > Radius)
				act.st = Move;
		}
		pCurrentPos = img;
}
	else
		act.st = Up;
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

	

	//for test//
	/*
	std::fstream out4("D:\\depthinfo.txt", std::ios::app);
	for (int i = 0; i < 424; i++) {
		for (int j = 0; j < 512; j++) {
			if (cv16img.at<USHORT>(i, j) > 0) {
				out4 << j << " " << i << " " << (float)(cv16img.at<USHORT>(i, j)) << std::endl;
			}
		}
	}
	std::cout << std::endl;
	*/
	//for test//
	
	

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

	std::fstream out11("D:\\Project.txt", std::ios::app);

	for (int i = 0; i < contour.size(); i++) {
		cv::Point2i pixelPoint = contour[i];
		USHORT depth = cv16Img.at<USHORT>(pixelPoint.y, pixelPoint.x);

		//test//
		/*
		pixelPoint.x = 283;
		pixelPoint.y = 129;
		depth = 1684;
		*/
		//test//



		V3f pKinectCoordinatePos(pixelPoint.x, pixelPoint.y, depth);
		pKinectCoordinatePos = getKinectPos(pKinectCoordinatePos);
		V3f pScreenProjectPos(pKinectCoordinatePos);
		   pScreenProjectPos = getProjectPos(pScreenProjectPos);
		V3f del = pScreenProjectPos - pKinectCoordinatePos; 
		float fdis = del.length();

		//打印五个点到平面的距离点//
		out11 << contour[i].x << " " << contour[i].y << " " << fdis << std::endl;


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

	cv::Point2f p, pLeft, pRight, pUp, pDown, pLeftUP, pLeftDown, pRightUp, pRightDown;
		for (int k = 0; k < contours.size(); k++) {
			 p = contours[k];
			 pLeft =cv::Point2f(p.x - 1, p.y);
			 pRight = cv::Point2f(p.x + 1, p.y);
			 pUp   =cv::Point2f(p.x, p.y - 1);
			 pDown  =cv::Point2f(p.x, p.y + 1);


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
					contours[k] = pneighbour[m];
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

	//对深度进行修正//
	USHORT depth1 = 0, depth2 = 0, depth3 = 0, depth4 = 0, depth5 = 0, depth6 = 0, depth7 = 0, depth8 = 0, depth9 = 0;
	float depth = 0;
	int Count = 1;

	std::vector<cv::Point3f> vOridinal;     //for test :在计算平均值的过程中是否存在坏点//

	    for (int j = 0; j < contours.size(); j++) {
			vOridinal.clear();

			p = contours[j];
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

			depth1 = cv16Img.at<USHORT>(contours[j].y, contours[j].x);
			depth = depth + depth1;
			vOridinal.push_back(Point3f(contours[j].x, contours[j].y, depth1));

			
			if (!isBorderOrOut(cv16Img,srcContours, pLeft)) {
				depth2 = cv16Img.at<USHORT>(contours[j].y, contours[j].x-1);
				depth = depth + depth2;
				Count++;

				vOridinal.push_back(Point3f(pLeft.x, pLeft.y, depth2));    //for test :在计算平均值的过程中是否存在坏点//

			}

			if (!isBorderOrOut(cv16Img,srcContours, pRight)) {
				depth3 = cv16Img.at<USHORT>(contours[j].y , contours[j].x+1);
				depth = depth + depth3;
				Count++;

				vOridinal.push_back(Point3f(pRight.x, pRight.y, depth3));  //for test :在计算平均值的过程中是否存在坏点//

			}
			if (!isBorderOrOut(cv16Img,srcContours, pUp)) {
				depth4 = cv16Img.at<USHORT>(contours[j].y-1, contours[j].x );
				depth = depth + depth4;
				Count++;

				vOridinal.push_back(Point3f(pUp.x, pUp.y, depth4));  //for test :在计算平均值的过程中是否存在坏点//

			}
			if (!isBorderOrOut(cv16Img,srcContours, pDown)) {
				depth5 = cv16Img.at<USHORT>(contours[j].y+1, contours[j].x );
				depth = depth + depth5;
				Count++;

				vOridinal.push_back(Point3f(pDown.x, pDown.y, depth5));  //for test :在计算平均值的过程中是否存在坏点//
			}


			//将平均值到8个方向，如果Count较少//
			if (Count < 3) {
				if (!isBorderOrOut(cv16Img, srcContours, pLeftUP)) {
					depth6 = cv16Img.at<USHORT>(contours[j].y - 1, contours[j].x - 1);
					depth = depth + depth6;
					Count++;

					vOridinal.push_back(Point3f(pLeftUP.x, pLeftUP.y, depth6));  //for test :在计算平均值的过程中是否存在坏点//

				}

				if (!isBorderOrOut(cv16Img, srcContours, pLeftDown)) {
					depth7 = cv16Img.at<USHORT>(contours[j].y + 1, contours[j].x - 1);
					depth = depth + depth7;
					Count++;
					vOridinal.push_back(Point3f(pLeftDown.x, pLeftDown.y, depth7)); //for test :在计算平均值的过程中是否存在坏点//
				}

				if (!isBorderOrOut(cv16Img, srcContours, pRightUp)) {
					depth8 = cv16Img.at<USHORT>(contours[j].y - 1, contours[j].x + 1);
					depth = depth + depth8;
					Count++;

					vOridinal.push_back(Point3f(pRightUp.x, pRightUp.y, depth8));  //for test :在计算平均值的过程中是否存在坏点//
				}

				if (!isBorderOrOut(cv16Img, srcContours, pRightDown)) {
					depth9 = cv16Img.at<USHORT>(contours[j].y + 1, contours[j].x + 1);
					depth = depth + depth9;
					Count++;

					vOridinal.push_back(Point3f(pRightDown.x, pRightDown.y, depth9));  //for test :在计算平均值的过程中是否存在坏点//
				}
			}
			//将平均值到8个方向，如果Count较少//


			//test//
			//在计算平均值的过程中是否会出现坏点的情况//

			//在计算平均值的过程中是否会出现坏点的情况//
			//test//


			depth = (depth / Count);
			cv16Img.at<USHORT>(contours[j].y, contours[j].x) = (USHORT)depth;
			depth = 0;
			Count = 1;


			//test:深度值改进的结果//
			std::fstream out16("D:\\depthrevise.txt", std::ios::app);
			for (int i = 0; i < contours.size(); i++) {
				out16 << contours[i].x << " " << contours[i].y << " " << cv16Img.at<USHORT>(contours[i].y, contours[i].x) << std::endl;
			}
			out16 << std::endl;
			out16 << std::endl;
			//test:深度值改进的结果//

	    }
	





		//test//
		//对深度值进行测试，看是否有坏点//
		std::fstream out20("D:\\checkDepth.txt", std::ios::app);
		for (int i = 0; i < vOridinal.size(); i++) {
			out20 << vOridinal[i] << std::endl;
		}
		out20 << std::endl;
		out20 << std::endl;


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
	cv::Point2f p, pLeft, pRight, pUp, pDown, pLeftUP, pLeftDown, pRightUp, pRightDown;
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
	cv::Point2f p(k, fMaxDepth);
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
	cv::Point2f p(k, fMinDepth);
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

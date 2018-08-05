#pragma once
#ifndef CCALIB_H
#define CCALIB_H
#include "dlpgesture/IDLPGesture.h"

//opencv //
#include "opencv/cv.h"
#include "opencv2/highgui/highgui.hpp"
#include "opencv/highgui.h"
using namespace cv;

//openextr//
#include "OpenExr/include/ImathVec.h"
#include "OpenExr/include/ImathLine.h"
#include "OpenExr/include/ImathLineAlgo.h"
#include "OpenExr/include/ImathPlane.h"
#include "OpenExr/include/ImathQuat.h"
using namespace IMATH_NAMESPACE;

//OPENCV//
#include "opencv/cv.h"
#include "opencv2/highgui/highgui.hpp"
#include "opencv/highgui.h"
using namespace cv;

using namespace GestureEng;


class CCalib : public ICalib, public CUnknown
{
public:
	DECLARE_IUNKNOWN
	STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void ** ppv);
	void GetRegZone(std::vector<nvmath::Vec2f> & vPixelPos);
	CCalib();
	~CCalib();

	void GetAllCounter(const int &colorwidth, const int & colorheight, const void* pColorBuffer);


	void SetScreenPos(const std::vector<nvmath::Vec3f> & vRegZonePixel, const std::vector<nvmath::Vec3f> & vPixelPos, std::vector<nvmath::Vec3f> & vKinectPos);
	void SetScreenPos_1(const std::vector<nvmath::Vec3f> &vpixelKinectPos, const std::vector<nvmath::Vec3f>&  vKinectPos);

private:
	bool                                                 Precalc16_fast(cv::Mat & cv16img, int ndepthWidth, int ndepthHeight);
	BOOL                                                 SeekHands(cv::Mat& cv16img, V3f & vHandCenterPos, const int &imgwidth, const int &                                                                imgheight); 
	void                                                 GetMinX(const std::vector<cv::Point> & counters, int & MinPixelXIndex);
	void                                                 GetMaxX(const std::vector<cv::Point> & counters, int & MaxPixelXIndex);
	void                                                 GetMinY(const std::vector<cv::Point> & counters, int & MinPixelYIndex);
	void                                                 GetMaxY(const std::vector<cv::Point> & counters, int & MaxPixelYIndeY);
	void                                                 GetProPixelPoints(cv::Mat& cv16img, V3f & vScreemPixelPos, int width, int height,                                                                          V3f & PixelPos);
	void                                                 GetAllScreenKinectPos(std::vector<V3f> & ScreenKinectPos);
	void                                                 GetCompensatePixelPoints(const V3f & v1, const V3f & v2, const V3f & v3, V3f & v4);
	void                                                 GetCounter(const int & colorwidth, const int & colorheight);
	void                                                 on_trackbar(int pos, std::vector<vector<Point>> & contours, const int &imgwidth,                                                                    const int & imgheight);
	void                                                 GetZoneRestrict();

	void                                                 GetColorMat(const int &colorwidth, const int & colorheight, const void*                                                                             pColorBuffer);

	void                                                Mult(const V3f &vec, V3f &v);
	bool                                                SetKinectCoordinateSystem();

	void                                                 GetInentZone();


	PIMathBase                                           m_PIMathBase;
	std::vector<V3f>                                     m_vMarkPoint;
	cv::Mat                                              m_cv8Img;
	std::vector<std::vector<Point>>                      m_contours;
	std::vector<cv::Point>                               m_CalibZoneCounter;

	//获取识别区域的深度以及对应的像素点//
	float                                                m_iMinLimitx_1 = 0;
	float                                                m_iMaxLimitx_1 = 0;
	float                                                m_iMaxLimity_1 = 0;
	float                                                m_iMinLimity_1 = 0;
	float                                                m_iMaxLimitz_1 = 0;
	float                                                m_iMinLimitz_1 = 0;

	//利用Kinect坐标系下的标定平面坐标对识别区域进行限制//
	float                                                m_iMaxLimitx   = 0;
	float                                                m_iMinLimitx   = 0;
	float                                                m_iMaxLimitz   = 0;
	float                                                m_iMinLimitz   = 0;

	//利用Kinect的标定平面确定手势的识别区域//
	float                                                 m_fMaxIdentX;  //标定板识别区域的阈值//
	float                                                 m_fMinIdentX;
	float                                                 m_fMaxIdentY;
	float                                                 m_fMinIdentY;


	std::vector<V3f>                                     m_vpixelKinectPos;
	std::vector<V3f>                                     m_vKinectPos;

	V3f                                                  m_vProToCalibPlanePos;         //将Kinect坐标点投影到标定平面坐标系下的点的坐标,只需要X,Y坐标//

	Plane3f                                              m_CalibPlane;

	Matrix44<float>                                      m_invTransFromMat;


	IplImage *                                           g_pGrayImage = NULL;
	IplImage *                                           g_pBinaryImage = NULL;

};


#endif // !CCALIB_H

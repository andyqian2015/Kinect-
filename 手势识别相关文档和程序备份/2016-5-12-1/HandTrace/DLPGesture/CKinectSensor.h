
#pragma once
#ifndef CKINECTSENSOR_H
#define CKINECTSENSOR_H

#include "dlpgesture/IDLPGesture.h"
#include "Kinect.h"
#include "DLPUtils/FileSystem.h"
#include "DLPUtils/Thread.h"
#include "DLPUtils/AdvCriticalSection.h"

#define  foc 370

//test//

#include <time.h>


class  CKinectSensor : public IMyKinectSensor, public IImgTransform,public CUnknown
{
public:
	DECLARE_IUNKNOWN
	STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void ** ppv);
	CKinectSensor();
	~CKinectSensor();

	//IKinectSensor
	BOOL Init(UINT uIndex = 0);
	BOOL Connect(PIDLPUnknown pRcv, BOOL bConnect, eKinectImage ImageType); //连接IKinectImage或者IKinectSke
	UINT GetCount(); //对于有多台Kinect设备的考虑//
	void Close();

	//IImgTransform
	BOOL ColorPixelToDepthPos(const void * depthBuffer, int depthwidth, int depthheight, const void* colorBuffer, int colorwidth, int colorheight, const sColorDiffIndex &colorDiffIndex, std::vector<nvmath::Vec2f> &CalibPixelPoints, std::vector<nvmath::Vec3f> &vMarkKinectPos);

	void SetCalibPixelPoints(std::vector<nvmath::Vec2f> & vCalibPixelPoints);

protected:
	struct SImageStruct {
		SImageStruct() :m_width(0), m_height(0), m_pitch(0),  m_uTimeStamp(0), m_pBuffer(NULL), m_kinectImageType(KI_None) {}
		PIUnknown                                             m_pIFrame;  
		void*                                                 m_pBuffer;
		int                                                   m_width;
		int                                                   m_height;
		int                                                   m_pitch;
		INT64                                                 m_uTimeStamp;
		eKinectImage                                          m_kinectImageType;
	} ;

	BOOL OpenDevice();

	void OnThread(int nEventID);

	BOOL GetColorImage(SImageStruct &ImgStruct);
	BOOL GetDepthImage(SImageStruct &ImgStruct);
	BOOL GetIRImage(SImageStruct &ImgStruct);

	BOOL GetColorHandle();
	BOOL GetDepthHandle();
	BOOL GetIRHandle();

	void ReviseAvgPixel(std::vector<nvmath::Vec3f> & validPoints, nvmath::Vec3f &avgPixel);

	void  getKinectPos(const int &ImgWidth, const int &ImgHeight,const nvmath::Vec3f& PixelPos, nvmath::Vec3f & KinectPos);

	//IImgTransform

	//V2
	//transform depth to color//
	BOOL ProcessFrame_1(const void* pDepthBuffer, int nDepthWidth, int nDepthHeight, const void* pColorBuffer, int nColorWidth, int nColorHeight, ICoordinateMapper*pCoordinateMapper, ColorSpacePoint* pColorCoordinates, nvmath::Vec2f & vColorPixelPoint);

	BOOL ProcessFrame_2(const void* pDepthBuffer, int nDepthWidth, int nDepthHeight, const void* pColorBuffer, int nColorWidth, int nColorHeight, ICoordinateMapper*pCoordinateMapper, DepthSpacePoint* DepthCoordinates, const std::vector<nvmath::Vec2f> & vColorPixelPoint, std::vector<nvmath::Vec3f> & vKinectPos);
	

	//V2

	void getPixelPos(const nvmath::Vec3f & pos, nvmath::Vec3f &pixelPoint, int DepthWidth, int DepthHeight);

	//depth - color - colorCenter//
	BOOL ProcessFrame_3(const void* pDepthBuffer, int nDepthWidth, int nDepthHeight, const void* pColorBuffer, int nColorWidth, int nColorHeight, ICoordinateMapper*pCoordinateMapper, DepthSpacePoint* DepthCoordinates, ColorSpacePoint* ColorCoordinates, const nvmath::Vec3f & vScreemDepthPixelPos, nvmath::Vec3f & CalibColorPixelPoints);

	BOOL GetKinectPos(const DepthSpacePoint p, const int &nDepthWidth, const int & nDepthHeight, const void* pDepthBuffer, nvmath::Vec3f & kinectPos);
	//V2

	BOOL ProcessFrame(const void* pDepthBuffer, int nDepthWidth, int nDepthHeight, const void* pColorBuffer, int nColorWidth, int nColorHeight, ICoordinateMapper*pCoordinateMapper, DepthSpacePoint* pDepthCoordinates, const sColorDiffIndex &colorDiffIndex, std::vector<nvmath::Vec3f> &vMarkKinectPos);

	//V3
	//利用获取的彩色标定图像的像素点获取标定点的深度值//
	BOOL ProcessFrame_4(const void* pDepthBuffer, int nDepthWidth, int nDepthHeight, const void* pColorBuffer, int nColorWidth, int nColorHeight, ICoordinateMapper*pCoordinateMapper,DepthSpacePoint* pDepthCoordinates, std::vector<nvmath::Vec3f> &vMarkKinectPos);

	BOOL GetDepth(ICoordinateMapper*pCoordinateMapper, DepthSpacePoint* pDepthCoordinates, const int & nColorWidth, const int & nColorHeight, nvmath::Vec2f & vpixelPoints, float &depth, const int & nDepthWidth, const int & nDepthHeight, const void* pDepthBuffer);


	std::vector<PIKinectImage>                                    m_ColorImageSink;
	std::vector<PIKinectImage>                                    m_DepthImageSink;
	std::vector<PIKinectImage>                                    m_IRImageSink;
	std::vector<PIKinectSkeleton>                                 m_SkeletonSink;

	typedef NSINSIVE_PTR::intrusive_ptr<IKinectSensor>            PIKinectSensor;
	typedef NSINSIVE_PTR::intrusive_ptr<IDepthFrameReader>        PIDepthFrameReader;
	typedef NSINSIVE_PTR::intrusive_ptr<IColorFrameReader>        PIColorFrameReader;
	typedef NSINSIVE_PTR::intrusive_ptr<IInfraredFrameReader>     PIInfraredFrameReader;

	PIKinectSensor                                                m_pKinectSensor;
	PIDepthFrameReader                                            m_pIDepthFrameReader;
	PIColorFrameReader                                            m_pIColorFrameReader;
	PIInfraredFrameReader                                         m_pIInfraredFrameReader;
                                                  
	std::vector<RGBQUAD>                                          m_vOutColorBuffer;

	int                                                           m_RegRadius;

	enum{
		EVENT_COLOR=0,
		EVENT_DEPTH,
		EVENT_IR,
	};
	THREAD::CThread                                               m_thread;
	THREAD::CAdvCriticalSection                                   m_cs;
	WAITABLE_HANDLE                                               m_waitColorHandle  ;
	WAITABLE_HANDLE	                                              m_waitDepthHandle  ; 
	WAITABLE_HANDLE                                               m_waitIRHandle     ;


	int                                                           m_test = 0;


	//V2
	PICalib                                                      m_PICalib;

	//在Kinect标定图像空间是逆时针标定//
	std::vector<nvmath::Vec2f>                                   m_CalibPixelPoints;

};

#endif
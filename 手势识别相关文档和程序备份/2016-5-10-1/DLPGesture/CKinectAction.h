#pragma once
#ifndef CKINECTACTION_H
#define CKINECTACTION_H

#include "dlpgesture\IDLPGesture.h"
#include <inc/Kinect.h>
#include <memory>
#include "DLPUtils/AdvCriticalSection.h"

#include <atlbase.h>

//for test
#include <time.h>
#include <fstream>
using namespace GestureEng;


class CKinectAction : public IKinectAction,  public IKinectSkeleton, public CUnknown
{
public:
	DECLARE_IUNKNOWN
	STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void **ppv);

	CKinectAction();
	~CKinectAction();

	class CImageRcv :public IKinectImage, public CUnknown
	{
	public:

		DECLARE_IUNKNOWN
		STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void **ppv)
		{
			if (riid == __uuidof(IKinectImage)) {
				return DLPGetInterface((IKinectImage*)this, ppv);
			}

			return __super::NonDelegatingQueryInterface(riid, ppv);
		}

		CImageRcv(CKinectAction*pThis) :m_pThis(pThis){}
		void OnImage(const void* pBuff, int nW, int nH, int nPitch, eKinectImage nImageType, UINT uTimestamp){
			m_pThis->OnImage(pBuff, nW, nH, nPitch, nImageType, uTimestamp);
		}
	protected:
		CKinectAction*m_pThis;
	};

	//IKinectImage
	void OnImage(const void* pBuff, int nW, int nH, int nPitch, eKinectImage nImageType, UINT uTimestamp);

	//IKinectSke
	void OnSkeleton();

	
	BOOL CKinectAction::Calib(const std::vector<nvmath::Vec3f> & vMarkScreemPoints, const std::vector<nvmath::Vec2f> &CalibPixelPoints);

	void StartReg();

	//IKinectActionEvent
	BOOL Connect(PIDLPUnknown pKinectActionEvent, bool bConnect);

	BOOL Init(PIDLPUnknown pKinectSensor, const std::vector<nvmath::Vec3f> & vMarkScreemPoints, const std::vector<nvmath::Vec3f> & vMarkKinectPoints);

	BOOL SetRegParam(const SGestureRegInitParam &sRegParam);

/*	void SetKinectInStallMode(eInstallMode InstallMode);*/
//	void SetColorDiffIndex(const sColorDiffIndex &colordiffindex);

	void GetCalibParam(std::vector<nvmath::Vec3f> & vMarkScreemPoints, std::vector<nvmath::Vec3f> & vMarkKinectPoints);

/*	void SetCalibImgPixelPoints(std::vector<nvmath::Vec2f> &CalibPixelPoints);*/


private:
	BOOL Calib_Inn(const void * depthBuffer, int depthwidth, int depthheight, const void* colorBuffer, int colorwidth, int colorheight, const sColorDiffIndex &colorDiffIndex);

	BOOL SetImageConnect();
	BOOL DisConnectImage(eKinectImage ImageType);

	BOOL SetCalib(const void* pBuff, int nW, int nH, int nPitch, eKinectImage nImageType, INT64 uTimestamp);
	void GetGesture(const void* pBuff, int nW, int nH, int nPitch, eKinectImage nImageType, INT64 uTimestamp);

	struct SKinectImage
	{
		SKinectImage() : m_width(0), m_height(0), m_pitch(0), m_imgType(KI_None), m_timeStamp(0){}
		std::shared_ptr<std::vector<byte>> m_Buffer_ptr;

		void*                    m_pBuffer;
		int                      m_width;
		int                      m_height;
		int                      m_pitch;
		int                      m_imgType;
		UINT64                   m_timeStamp;
	}  ;

	std::vector<sAction1>                   m_HandAction;

	PIKinectImage                           m_pImage;

	PIMyKinectSensor                        m_pMyKinect;
	PIImgTransform                          m_pIImgTransform;
	PIHandTrace                             m_PIHandTrace;

	sColorDiffIndex                         m_ColorDiffIndex;

	std::vector<PIKinectActionEvent>        m_HandEventSink;

	std::vector<UINT16>                     m_vdepthBuffer;
	std::vector<RGBQUAD>                    m_vcolorBuffer;



	BOOL                                    m_bCalib;
	BOOL                                    m_bRegGesture;
	BOOL                                    m_bInit ;

	int                                     m_ColorOpenCount = 0;  //彩色帧图像连接的次数//
	int                                     m_DepthOpenCount = 0;  //深度帧图像连接的次数//
	int                                     m_IROpenCount    = 0;  //红外帧图像连接的次数//

	std::vector<SKinectImage>               m_vdepthImg;
	std::vector<SKinectImage>               m_vcolorImg;

	std::vector<nvmath::Vec3f>              m_vMarkKinectPos;
	std::vector<nvmath::Vec3f>              m_vMarkScreemPoints;

	std::vector<nvmath::Vec2f>              m_CalibPixelPoints;

	SGestureRegInitParam                    m_sRegParam;

	eInstallMode                            m_InstallMode = Forward;
	
	BOOL                                    m_bdatainit;

	THREAD::CAdvCriticalSection             m_cs;

	BOOL                                    m_bReg = false;

};

#endif // !CKINECTACTION_H
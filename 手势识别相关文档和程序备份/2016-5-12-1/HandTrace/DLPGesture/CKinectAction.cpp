#include "stdafx.h"
#include "CKinectAction.h"

	STDMETHODIMP CKinectAction::NonDelegatingQueryInterface(REFIID riid, void **ppv)
	{
		if (riid == __uuidof(IKinectAction))	{
			return DLPGetInterface((IKinectAction*)this, ppv);
		}

		if (riid == __uuidof(IKinectSkeleton))	{
			return DLPGetInterface((IKinectSkeleton*)this, ppv);
		}

		return __super::NonDelegatingQueryInterface(riid, ppv);
	}

	CKinectAction::CKinectAction() : m_bCalib(false), m_bRegGesture(false),  m_bInit(false),m_bdatainit(true)
	{
		
		m_pImage = PIKinectImage(new CImageRcv(this));

		PIDLPUnknown p  = CreateHandObj(__uuidof(IHandTrace));
		m_PIHandTrace   = GetDLPUnknown<IHandTrace>(p);
	}


	CKinectAction::~CKinectAction()
	{
		if (m_pMyKinect)
		{
			DisConnectImage(KI_COLOR);
			DisConnectImage(KI_Depth);
			DisConnectImage(KI_IR);
		}
    }

	void CKinectAction::OnSkeleton() 
	{
		THREAD::CAutoCriticalSection cs(m_cs);
		return;
	}

	BOOL CKinectAction::Calib(const std::vector<nvmath::Vec3f> & vMarkScreemPoints)
	{
		if (vMarkScreemPoints.size() != 4)
			return  false;

			m_bCalib = true;
			m_vMarkScreemPoints = vMarkScreemPoints;
// 			nvmath::Vec3f vec1 = vMarkScreemPoints[0] - vMarkScreemPoints[1];
// 			nvmath::Vec3f vec2 = vMarkScreemPoints[2] - vMarkScreemPoints[1];
// 			nvmath::Vec3f marksCreenPos = vec1 + vec2 + vMarkScreemPoints[1];
// 			m_vMarkScreemPoints.push_back(marksCreenPos); 
			return true;
	}


	BOOL CKinectAction::SetCalib(const void* pBuff, int nW, int nH, int nPitch, eKinectImage nImageType, INT64 uTimestamp)
	{
		if (m_vMarkScreemPoints.size() != 4)
			return false;

		if (m_vcolorImg.empty() && nImageType == KI_COLOR)
		{
			m_vcolorBuffer.resize(nW *nH);

			for (int i = 0; i < nH * nW; ++i) {
				m_vcolorBuffer[i] = *((RGBQUAD*)(pBuff)+i );
			}

			m_vcolorImg.resize(1);
			
			m_vcolorImg[0].m_pBuffer = &m_vcolorBuffer[0];
			m_vcolorImg[0].m_height = nH;
			m_vcolorImg[0].m_width = nW;
			m_vcolorImg[0].m_imgType = nImageType;
			m_vcolorImg[0].m_timeStamp = uTimestamp;
			m_vcolorImg[0].m_pitch = nPitch;

		}


		if (m_vdepthImg.empty() && nImageType == KI_Depth) {

			m_vdepthBuffer.resize(nW *nH);

			for (int i = 0; i < nH * nW; ++i) {
				m_vdepthBuffer[i] = *((UINT16*)(pBuff)+i );
			}


			m_vdepthImg.resize(1);
			m_vdepthImg[0].m_pBuffer = &m_vdepthBuffer[0];
			m_vdepthImg[0].m_width = nW;
			m_vdepthImg[0].m_height = nH;
			m_vdepthImg[0].m_imgType = nImageType;
			m_vdepthImg[0].m_timeStamp = uTimestamp;
			m_vdepthImg[0].m_pitch = nPitch;
		}


		if (m_vcolorImg.empty() || m_vdepthImg.empty())
			return false;

		m_bdatainit = false;
		BOOL bSuccesCalib_inn = false;

		bSuccesCalib_inn = Calib_Inn(m_vdepthImg[0].m_pBuffer, m_vdepthImg[0].m_width, m_vdepthImg[0].m_height, m_vcolorImg[0].m_pBuffer, m_vcolorImg[0].m_width, m_vcolorImg[0].m_height, m_ColorDiffIndex);

		if (bSuccesCalib_inn)
			m_bCalib = false;

		////清空掉标定的彩色图像和深度图像
		m_vcolorImg.clear();
		m_vcolorBuffer.clear();
		m_vdepthImg.clear();
		m_vdepthBuffer.clear();

		return bSuccesCalib_inn;
	}


	void CKinectAction::GetGesture(const void* pBuff, int nW, int nH, int nPitch, eKinectImage nImageType, INT64 uTimestamp)
	{
		if (!m_bdatainit)
		{
			if (nImageType != KI_Depth)
				return;
			m_bdatainit = m_PIHandTrace->InitData(nW,nH,m_vMarkScreemPoints, m_vMarkKinectPos, m_InstallMode, m_sRegParam.h, m_sRegParam.h1, m_sRegParam.Radius);
			m_bdatainit = true;
		}

		if (!m_bdatainit || nImageType != KI_Depth)
			return;

		BOOL bGetHand = m_PIHandTrace->GetHandGesture(pBuff, nW, nH, m_HandAction, uTimestamp);

		if (!bGetHand)
			return;

		for (auto it = m_HandEventSink.begin(); it != m_HandEventSink.end(); ++it) {
			(*it)->OnAction(m_HandAction);
			
		}
			
	}

	void CKinectAction::OnImage(const void* pBuff, int nW, int nH, int nPitch, eKinectImage nImageType, UINT uTimestamp)
	{
		THREAD::CAutoCriticalSection cs(m_cs);

		if (m_bCalib) {
			BOOL bSuccesCalib = SetCalib(pBuff, nW, nH, nPitch, nImageType, uTimestamp);
			if (!bSuccesCalib)
				return;
		}
		
		if (m_vMarkKinectPos.size() != 4)
			return;
		GetGesture(pBuff, nW, nH, nPitch, nImageType, uTimestamp);
	}


	BOOL CKinectAction::Calib_Inn(const void * depthBuffer, int depthwidth, int depthheight, const void* colorBuffer, int colorwidth, int colorheight, const sColorDiffIndex &colorDiffIndex)
 	{
		if (!m_bInit || !m_pIImgTransform)
			return false;


		BOOL bCalibSucces = m_pIImgTransform->ColorPixelToDepthPos(depthBuffer, depthwidth, depthheight, colorBuffer, colorwidth, colorheight, colorDiffIndex, m_CalibPixelPoints, m_vMarkKinectPos);

		return bCalibSucces;
	}


	void CKinectAction::SetCalibImgPixelPoints(std::vector<nvmath::Vec2f> &CalibPixelPoints)
	{
		m_CalibPixelPoints = CalibPixelPoints;
	}

	void CKinectAction::SetKinectInStallMode(eInstallMode Mode)
	{
		m_InstallMode = Mode;
		m_bCalib      = true;
		m_vcolorImg.clear();
		m_vdepthImg.clear();
	}
	//IKinectActionEvent
	BOOL CKinectAction::Connect(PIDLPUnknown pIKinectActionEvent, bool bConnect) {

		PIKinectActionEvent p = GetDLPUnknown<IKinectActionEvent>(pIKinectActionEvent);
		
		if (!p)
			return false;

		if (bConnect)
		{
			if (p)
				m_HandEventSink.push_back(p);
		}

		else {
			for (auto it = m_HandEventSink.begin(); it != m_HandEventSink.end(); ++it)
			{
				if (*it == p)
					it = m_HandEventSink.erase(it);

				if (m_HandEventSink.empty())
					return true;
			}
		}
		return true;
	}


	void CKinectAction::GetCalibParam(std::vector<nvmath::Vec3f> & vMarkScreemPoints, std::vector<nvmath::Vec3f> & vMarkKinectPoints)
	{
		vMarkScreemPoints = m_vMarkScreemPoints;
		vMarkKinectPoints = m_vMarkKinectPos;
	}


	BOOL CKinectAction::Init(PIDLPUnknown pKinectSensor, const std::vector<nvmath::Vec3f> & vMarkScreemPoints, const std::vector<nvmath::Vec3f> & vMarkKinectPoints)
	{
		if (!pKinectSensor)
			return false;

		m_pMyKinect = GetDLPUnknown<IMyKinectSensor>(pKinectSensor);
		m_pIImgTransform = GetDLPUnknown<IImgTransform>(pKinectSensor);

		m_vMarkScreemPoints = vMarkScreemPoints;
		m_vMarkKinectPos    = vMarkKinectPoints;

		if (m_vMarkScreemPoints.size() != 4 && m_vMarkKinectPos.size() != 4)
			m_bCalib = true;

		if (SetImageConnect())
			m_bInit = true;

		return m_bInit;
	}

	BOOL CKinectAction::SetRegParam(const SGestureRegInitParam &sRegParam)
	{
		m_sRegParam = sRegParam;
		m_bdatainit = false;
		return true;
	}

	

  void CKinectAction::SetColorDiffIndex(const sColorDiffIndex &colordiffindex)
  {
	  m_ColorDiffIndex = colordiffindex;
	  m_bCalib         = true;
	  m_vcolorImg.clear();
	  m_vdepthImg.clear();
  }

  BOOL CKinectAction::SetImageConnect()
  {
	  if (!m_pImage || !m_pMyKinect)
		  return false;

	  BOOL bdepthImgOpen = m_pMyKinect->Connect(m_pImage, true, KI_Depth);
	  BOOL bcolorImgOpen = m_pMyKinect->Connect(m_pImage, true, KI_COLOR);

	  if (bcolorImgOpen)
		  m_ColorOpenCount++;

	  if (bdepthImgOpen)
		  m_DepthOpenCount++;

	  return (bdepthImgOpen && bcolorImgOpen);
  }

  BOOL CKinectAction::DisConnectImage(eKinectImage ImageType)
  {
	  if (!m_pMyKinect)
		  return false;

	  while (m_ColorOpenCount)  
	  {
		  bool bDisConnect = m_pMyKinect->Connect(m_pImage, false, ImageType);
		  if (bDisConnect)
			  m_ColorOpenCount--;
	  }
	  return true;
  }

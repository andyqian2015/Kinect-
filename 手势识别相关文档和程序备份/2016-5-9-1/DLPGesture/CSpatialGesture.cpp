#define SPTGST_DLL
#include "stdafx.h"
#include "CSpatialGesture.h"
#include "CLeftGrip.h"
#include "CRightGrip.h"

namespace SpatialGestureEng
{
	PIStartSpatialGesture StartSpatialGesture()
	{
		PIStartSpatialGesture p(new SpatialGestureEng::CAdvancedGesture);
		return p;
	}


	PISpatialGesture CreateLeftHand()
	{
		PISpatialGesture p(new CLeftGrip);
		return p;
	}

	PISpatialGesture CreateRightHand()
	{
		PISpatialGesture p(new CRightGrip);
		return p;
	}

	CAdvancedGesture::CAdvancedGesture()
	{
		sGesture s;
		s.pEng = NULL;
		s.bEnable = false;
		m_Gestures.push_back(s);
		m_Gestures.push_back(s);
		PISpatialGesture p;
		m_Gestures[0].pEng = CreateLeftHand();
		m_Gestures[1].pEng = CreateRightHand();

		m_pKinectSensor = nullptr;

		HRESULT hr;

		hr = GetDefaultKinectSensor(&m_pKinectSensor);

		if (FAILED(hr))
		{
			return;
		}

		m_pCoordinateMapper = nullptr;
		m_pBodyFrameReader = nullptr;

		// Initialize the Kinect and get coordinate mapper and the body reader  
		if (m_pKinectSensor)
		{
			IBodyFrameSource* pBodyFrameSource = nullptr;

			hr = m_pKinectSensor->Open();

			if (SUCCEEDED(hr))
			{
				hr = m_pKinectSensor->get_CoordinateMapper(&m_pCoordinateMapper);
			}
			//����get_CoordinateMapper�õ�����ӳ��
			if (SUCCEEDED(hr))
			{
				hr = m_pKinectSensor->get_BodyFrameSource(&pBodyFrameSource);
			}
			//����get_BodyFrameSource�õ�����֡Դ
			if (SUCCEEDED(hr))
			{
				hr = pBodyFrameSource->OpenReader(&m_pBodyFrameReader);
			}
			SafeRelease(pBodyFrameSource);
			//����get_BodyFrameSource�򿪹���֡��ȡ��

			if (!m_pBodyFrameReader)
			{
				return;
			}
		}
		if (!m_pKinectSensor || FAILED(hr))
		{
			printf("No ready Kinect found!");
			return;
		}
	}

	void CAdvancedGesture::OnAction(std::vector<SpatialGesture> &sg)
	{
		if (!m_pBodyFrameReader)
		{
			return;
		}

		IBodyFrame *pBodyFrame = nullptr;
		//���¹���֡
		HRESULT hr_reader = m_pBodyFrameReader->AcquireLatestFrame(&pBodyFrame);
		//���¹�������  
		if (SUCCEEDED(hr_reader))
		{
			IBody *pBody[BODY_COUNT] = { 0 };
			HRESULT hResult = pBodyFrame->GetAndRefreshBodyData(_countof(pBody), pBody);
			// 			if (FAILED(hResult))
			// 			{
			// 				return;
			// 			}
			if (SUCCEEDED(hResult))
			{
				//����pBody

				for (UINT i = 0; i != m_Gestures.size(); i++)
				{
					if (!m_Gestures[i].bEnable)
					{
						if (m_Gestures[i].pEng)
						{
							SpatialGesture s;
							TIMESPAN t=0;
							s.time = 0;
							hResult = pBodyFrame->get_RelativeTime(&t);
							if (SUCCEEDED(hResult))
							{
								s.time = t;
							}
							m_Gestures[i].pEng->OnAction(pBody,s);
							if (s.type!=None)
							{
								sg.push_back(s);
							}
						}
					}
				}
			}
			for (UINT i = 0; i < _countof(pBody); ++i)
			{
				SafeRelease(pBody[i]);
			}
		}

		SafeRelease(pBodyFrame);		
	}

	void CAdvancedGesture::EnableGesture(GestureType type, bool bEnable)
	{

	}
}

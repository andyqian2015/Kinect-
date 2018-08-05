#include "stdafx.h"
#include "CSynGesture.h"

STDMETHODIMP CSynGesture::NonDelegatingQueryInterface(REFIID riid, void **ppv)
{
	if (riid == __uuidof(GestureEng::ISynGesture))
		return DLPGetInterface((GestureEng::ISynGesture*)this, ppv);
	return __super::NonDelegatingQueryInterface(riid, ppv);
}

CSynGesture::CSynGesture()
{
	//设置KienctAction连接: 主要是通过调用KinectAction获取到基本的手势//
	PIDLPUnknown p = CreateHandObj(__uuidof(IKinectAction));
	m_pkinectAction = GetDLPUnknown<IKinectAction>(p);

	m_pmykinectSensor = CreateHandObj(__uuidof(IMyKinectSensor));
	if (!m_mykiectSensor)
		m_mykiectSensor = GetDLPUnknown<IMyKinectSensor>(m_pmykinectSensor);

	PIDLPUnknown p2 = CreateHandObj(__uuidof(IEvent));
	m_PIEvent = GetDLPUnknown<IEvent>(p2);
// 	m_func = std::bind(&CSynGesture::GetGesture,this, std::placeholders::_1);
// 	m_PIEvent->Registry(m_func);
	m_PIEvent->Init(p);

}

BOOL CSynGesture::InitDevice()
{
	if (!m_mykiectSensor)
		return false;

	return m_mykiectSensor->Init();
}

BOOL CSynGesture::InitGesReg(const std::vector<nvmath::Vec3f> & vMarkScreemPoints, const std::vector<nvmath::Vec3f> & vMarkKinectPoints)
{
	if (!m_pkinectAction || !m_pmykinectSensor)
		return false;
	return m_pkinectAction->Init(m_pmykinectSensor, vMarkScreemPoints, vMarkKinectPoints);
}

BOOL CSynGesture::Calib(const std::vector<nvmath::Vec3f> & vMarkScreemPoints, const std::vector<nvmath::Vec2f> &CalibPixelPoints)
{
	if (!m_pkinectAction)
		return false;
	return m_pkinectAction->Calib(vMarkScreemPoints, CalibPixelPoints);
}


// void CSynGesture::GetGesture(std::vector<sGesture> g)
// {
// 	int testm = 10;
// 
// 	m_fun(g);
// }


// void CSynGesture::Registry(mycallbackfunc  callbackfunc)
// {
// 	m_fun = callbackfunc;
// }

BOOL CSynGesture::SetEnableGesture(eGestureType gtype, bool bEnable)
{
	m_PIEvent->SetEnableGesture(gtype, bEnable);
	return true;
}

void CSynGesture::StartReg()
{
	m_pkinectAction->StartReg();
}

void CSynGesture::SetGestureConnect(PISendSynGesture pGesture)
{
	m_PIEvent->SetGestureConnect(pGesture);
}






#include "stdafx.h"
#include "CEvent.h"

#include <fstream>

STDMETHODIMP CEvent::NonDelegatingQueryInterface(REFIID riid, void **ppv)
{
	if (riid == __uuidof(GestureEng::IEvent)){
		return DLPGetInterface((GestureEng::IEvent*)this, ppv);
	}

	return __super::NonDelegatingQueryInterface(riid, ppv);
}

void CEvent::OnAction(const std::vector<sAction> & saction, std::vector<sGesture> g)
{

// 	nvmath::Vec2f v1(352.426, 374.327), v2(152.763,353.809), v11(352.402, 372.451), v22(149.983, 360.975);
// 	nvmath::Vec2f dv2v1 = v2 - v1;
// 	nvmath::Vec2f dv4v3 = v22 - v11;
// 
// 	float angle = atan2(dv2v1[1], dv2v1[0]) /** 180 / 3.141592654*/;
// 	float angle1 = atan2(dv4v3[1], dv4v3[0]) /** 180 / 3.141592654*/;
// 
// 	float dAngle = (angle1 - angle) /** 180 / 3.141592654*/;
// 	float dAngle1 = (angle1 - angle) * 180 / 3.141592654;

	if (!m_pGesture)
		return;
	
	//test//
	if (saction.size() > 0)
	{
		int testm = 10;
	}
	if (saction.size() > 1 && (/*saction[1].st == Down||*/saction[1].st==Move))
		int Test10 = 10;

	m_pGesture->OnAction(saction, g);
	m_funcallback(g);

	//test//
	if (!g.empty())
	{
		for (int i = 0; i < g.size(); i++) {
			if (g[i].gtype == GRotate) {
				float fangle = g[i].fScreenRotateAngle;
				int testm = 10;
			}

		}
	}
	


	//test//
	if (!g.empty())
		for (int i = 0; i != g.size(); ++i)
		{
			if (g[i].gtype == 6 || 7 == g[i].gtype || 8 == g[i].gtype || 9 == g[i].gtype)
				int test = 0;
		}
}

BOOL CEvent::SetEventConnect()
{
	if (!m_PIKinectAction)
		return  false;

	BOOL bConnect = m_PIKinectAction->Connect(m_PIKinectActionEvent, true);
	if (bConnect)
		m_ActionEventCount++;
	return bConnect;
}

BOOL CEvent::Init(PIDLPUnknown pKinectAction)
{
	m_PIKinectAction = GetDLPUnknown<IKinectAction>(pKinectAction);

	if (!SetEventConnect())
		return false;

	return true;
}

CEvent::CEvent()
{
	if (!m_pGesture)
		m_pGesture = CreateGesture();

	m_PIKinectActionEvent = PIKinectActionEvent(new CKinectActionEvent(this));
}

CEvent::~CEvent() {

}

// void CEvent::GetGesture()
// {
// 	std::vector<sAction1>  saction;
// 
// 	m_PIKinectActionEvent->GetHands(saction);
// 
// 	 //test//
// 	 std::fstream outfile("D:\\test.txt", std::ios::app);
// 	 for (int i = 0; i < saction.size(); i++) {
// 		 outfile << saction[i].ID << " " << saction[i].pos.x << " " << saction[i].pos.y << " " << saction[i].pos.z << std::endl;
// 	 }
// 	 //test//
// 	 std::vector<sAction> p;
// //	 = saction;
// 
// 	 m_pGesture->OnAction(p, m_vHandGesture);
// 	 m_funcallback(m_vHandGesture);
// }

// void CEvent::Test(sGesture &s)
// {
// 	if (!m_vHandGesture.empty()) 
// 		s = m_vHandGesture[m_vHandGesture.size() - 1];
// 
// 	else
// 		s = sGesture();
// 
// 	m_vHandGesture.clear();
// }

BOOL CEvent::SetEnableGesture(eGestureType gtype, bool bEnable)
{
	if (!m_pGesture)
		return false;

	m_pGesture->EnableGesture(gtype, bEnable);

	GestureEng::sGestureParamFlip s;
	s.distance_flip = 80;
	s.type = GFlipLeft;
	if (gtype == GFlipLeft || gtype == GFlipRight || gtype == GFlipDown || gtype == GFlipUp)
	  m_pGesture->SetGestureParam(&s);

	GestureEng::sGestureParamSlideBack s1;
	s1.type = GSwithHori;
	s1.distance_slide = 80;
	s1.distance_back = 80;

	if (gtype == GSwithHori || gtype == GSwithVert)
		m_pGesture->SetGestureParam(&s1);

	GestureEng::sGestureParamDClick s2;
	s2.type = GDoubleClick;
	s2.time_dclick = 4000;
	s2.radius_dclick = 1000;
	if (gtype == GDoubleClick)
		m_pGesture->SetGestureParam(&s2);

}

void CEvent::Registry(mycallbackfunc callbackfunc)
{
	m_funcallback = callbackfunc;
}




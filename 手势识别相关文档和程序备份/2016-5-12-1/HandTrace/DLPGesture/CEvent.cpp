#include "stdafx.h"
#include "CEvent.h"

#include <fstream>

STDMETHODIMP CEvent::NonDelegatingQueryInterface(REFIID riid, void **ppv)
{
	if (riid == __uuidof(IEvent)){
		return DLPGetInterface((IEvent*)this, ppv);
	}

	return __super::NonDelegatingQueryInterface(riid, ppv);
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

	if (!m_pGesture)
		m_pGesture = CreateGesture();

	return true;
}

CEvent::CEvent()
{
	PIDLPUnknown p = CreateHandObj(__uuidof(IKinectActionEvent));
	m_PIKinectActionEvent = GetDLPUnknown<IKinectActionEvent>(p);
}

CEvent::~CEvent() {

}

void CEvent::GetGesture()
{
	std::vector<sAction1>  sAction;

	 m_PIKinectActionEvent->GetHands(sAction);

	 //test//
	 std::fstream outfile("D:\\test.txt", std::ios::app);
	 for (int i = 0; i < sAction.size(); i++) {
		 outfile << sAction[i].ID << " " << sAction[i].pos.x <<" " <<  sAction[i].pos.y << " " << sAction[i].pos.z << std::endl;
	 }
	 //test//


	for (int i = 0; i < sAction.size(); i++) {
		m_pGesture->OnAction(sAction[i], m_vHandGesture);
	}
}

void CEvent::Test(sGesture &s)
{
	if (!m_vHandGesture.empty()) 
		s = m_vHandGesture[m_vHandGesture.size() - 1];

	else
		s = sGesture();

	m_vHandGesture.clear();
}

BOOL CEvent::SetEnableGesture(eGestureType gtype, bool bEnable)
{
	if (!m_pGesture)
		return false;

	m_pGesture->EnableGesture(gtype, bEnable);
}


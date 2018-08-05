#include "stdafx.h"
#include "CKinectActionEvent.h"


	
// STDMETHODIMP CKinectActionEvent::NonDelegatingQueryInterface(REFIID riid, void **ppv) 
// {
// 		if (riid == __uuidof(IKinectActionEvent))
// 		{
// 			return DLPGetInterface((IKinectActionEvent*)this, ppv);
// 		}
// 
// 		return __super::NonDelegatingQueryInterface(riid, ppv);
// }

void CKinectActionEvent::OnAction(const std::vector<sAction1>& a)
{
	m_HandAction = a;
}

CKinectActionEvent::CKinectActionEvent()
{
	
}

void  CKinectActionEvent::GetHands(std::vector<sAction1> & sHandActions)
{
	sHandActions = m_HandAction;
}

CKinectActionEvent::~CKinectActionEvent()
{

}



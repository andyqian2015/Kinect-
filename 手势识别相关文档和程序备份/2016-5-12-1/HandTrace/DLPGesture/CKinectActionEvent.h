#include "dlpgesture\IDLPGesture.h"

class CKinectActionEvent : public IKinectActionEvent, public CUnknown
{
public:
	DECLARE_IUNKNOWN
/*	STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void **ppv);*/

	STDMETHODIMP CKinectActionEvent::NonDelegatingQueryInterface(REFIID riid, void **ppv)
	{
		if (riid == __uuidof(IKinectActionEvent))
		{
			return DLPGetInterface((IKinectActionEvent*)this, ppv);
		}

		return __super::NonDelegatingQueryInterface(riid, ppv);
	}

	CKinectActionEvent();
	~CKinectActionEvent();

	void OnAction(const std::vector<sAction1>& a);

	void GetHands(std::vector<sAction1> & sHandActions);

private:
	std::vector<sAction1>          m_HandAction;
};
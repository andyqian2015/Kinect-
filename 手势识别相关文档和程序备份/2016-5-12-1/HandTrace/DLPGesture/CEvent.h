#include "dlpgesture\IDLPGesture.h"

class CEvent : public IEvent, public CUnknown
{
public:
	DECLARE_IUNKNOWN
	STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void **ppv);
	BOOL               SetEventConnect();
	BOOL               Init(PIDLPUnknown pKinectAction);
	void               GetGesture();

	BOOL               SetEnableGesture(eGestureType gtype, bool bEnable);

	//test//
	void Test(sGesture &s);


	CEvent();
	~CEvent();

private:
	PIKinectAction                   m_PIKinectAction;
	PIKinectActionEvent              m_PIKinectActionEvent;
	int                              m_ActionEventCount = 0;  //����ʱ��ӿ����ӵĴ���//
	std::vector<sGesture>            m_vHandGesture;

	PIGesture                        m_pGesture; 


};
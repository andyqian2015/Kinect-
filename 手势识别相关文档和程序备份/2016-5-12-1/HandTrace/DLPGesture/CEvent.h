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
	int                              m_ActionEventCount = 0;  //手势时间接口连接的次数//
	std::vector<sGesture>            m_vHandGesture;

	PIGesture                        m_pGesture; 


};
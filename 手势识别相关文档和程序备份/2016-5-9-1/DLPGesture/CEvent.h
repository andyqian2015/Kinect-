#include "dlpgesture\IDLPGesture.h"

#include "CGstEnsenssial.h"

using namespace GestureEng;


class CEvent : public IEvent, public CUnknown
{
public:
	DECLARE_IUNKNOWN
	STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void **ppv);

	class CKinectActionEvent : public IKinectActionEvent, public CUnknown
	{
	public :
		DECLARE_IUNKNOWN
		STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void **ppv)
		{
			if (riid == __uuidof(GestureEng::IKinectActionEvent)){
				return DLPGetInterface((GestureEng::IKinectActionEvent*)this, ppv);
			}

			return __super::NonDelegatingQueryInterface(riid, ppv);
		}

		CKinectActionEvent(CEvent*pThis)
		{
			m_PIEvent = pThis;
		}

		void OnAction(const std::vector<sAction1>&  saction)
		{
			std::vector<sGesture> g;
			std::vector<sAction> s(saction.size());
			for (int i = 0; i < s.size(); i++) {
				s[i] = saction[i];
			}
			m_PIEvent->OnAction(s, g);	
		}

	protected:
		CEvent*                      m_PIEvent;

	};

	void Registry(mycallbackfunc callbackfunc);


	void OnAction(const std::vector<sAction> & sAction, std::vector<sGesture> g);


	BOOL               SetEventConnect();
	BOOL               Init(PIDLPUnknown pKinectAction);
/*	void               GetGesture();*/

	BOOL               SetEnableGesture(eGestureType gtype, bool bEnable);

	//test//
/*	void Test(sGesture &s);*/


	CEvent();
	~CEvent();

private:
	PIKinectAction                   m_PIKinectAction;
	PIKinectActionEvent              m_PIKinectActionEvent;
	int                              m_ActionEventCount = 0;  //手势时间接口连接的次数//
	std::vector<sGesture>            m_vHandGesture;

	PIGesture                        m_pGesture; 

	GestureEng::mycallbackfunc       m_funcallback;


};
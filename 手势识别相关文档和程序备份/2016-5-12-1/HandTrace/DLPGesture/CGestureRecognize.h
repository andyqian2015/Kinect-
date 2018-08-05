//创建可实例化的继承手势识别接口类
#pragma once

#include "InnerStructure.h"
#include "CGClick.h"
#include "CGDoubleClick.h"
#include "CGDrag.h"
#include "CGFlipUp.h"
#include "CGFlipDown.h"
#include "CGFlipLeft.h"
#include "CGFlipRight.h"
#include "CGLongPush.h"
#include "CGMoveTo.h"
#include "CGNone.h"
#include "CGSlideBackHori.h"
#include "CGSlideBackVert.h"
#include "CGZooming.h"
#include "CGRotate.h"
#include "CGstEnsenssial.h"

//test//
#include <fstream>
//test//

#ifndef GSTRCN_H
#define GSTRCN_H


namespace GestureEng
{
	class  CGestureRecognize : public IGesture
	{
	public:
		CGestureRecognize();
		~CGestureRecognize();

		//V2
		//增加一个内部类，用于获取基本的手势//
// 		class CKinectActionEvent : public IKinectActionEvent, public CUnknown
// 		{
// 		public:
// 			DECLARE_IUNKNOWN
// 			STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void **ppv)
// 			{
// 				if (riid == __uuidof(IKinectActionEvent))
// 					return DLPGetInterface((IKinectActionEvent*)this, ppv);
// 				return   __super::NonDelegatingQueryInterface(riid, ppv);
// 			}
// 
// 			CKinectActionEvent(CGestureRecognize* pThis) :m_pThis(pThis) {}
// 
// 			void OnAction(const std::vector<sAction1>&  sAction) {
// 				for (int i = 0; i < sAction.size(); i++)
// 				{
// 					m_pThis->OnAction(sAction[i], m_g);
// 					if (!m_g.empty()) {
// 						if (m_g.back().gtype == 15)
// 							m_g.clear();
// 					}
// 
// 				}				
// 			}
// 		protected:
// 			CGestureRecognize* m_pThis;
// 
// 			std::vector<sGesture>                 m_g;
// 			
// 		};
//V2
		
		//V2
		//test//
// 		void Test(sGesture &s);
// 		void init(PIDLPUnknown pKinectActon);
		//V2
		

		void				OnAction(const sAction &r, std::vector<sGesture> &g);
		int					SetGestureMode(const sGestureParam *gc);
		void				EnableGesture(eGestureType gtype, bool bEnable);
		void				Reset();
		bool				GetChecked(eGestureType type);

		const sGestureParam*				GetParamValue(eGestureType type);
		const sGestureParam*				GetBasicParam();
		const sGestureParamClick*			GetClickParam();
		const sGestureParamDClick*			GetDClickParam();
		const sGestureParamFlip*			GetFlipParam();
		const sGestureParamSlideBack*		GetBackParam();
		const sGestureParamZoomARoatate*	GetZARParam();

	private:
		void				ResetFlag();
		void				SetFlag();
		void				CleanQue(std::vector<sGesture> &g);


		//V2
// 		void               AdjustHandGesture(std::vector<sGesture> &handGesture);
// 		BOOL               SetEventConnect();   
// 		void               DisEventConnect();
// 
// 		PIKinectAction                   m_PIKinectAction;
// 		PIKinectActionEvent              m_PIKinectActionEvent;
// 		int                              m_ActionEventCount = 0;  //手势时间接口连接的次数//
// 		std::vector<sGesture>            m_vHandGesture;

		//V2

		


		GestureEng::SHANDGST			m_sg_1st, m_sg_2nd;
		sGesture						m_sg_over;
		GestureEng::GSTFLAG				m_gstflag;

	


		struct eGesture{
			PCGstEnsenssial		pEng;
			bool				bEnable;
		};
		std::vector<eGesture>	m_Gestures;
	};
}
#endif 
#pragma once

#include "InnerStructure.h"
#include "DLPGesture/GestureParam.h"
#include "CHand.h"




#ifndef CGSTENSENSSIAL_H
#define CGSTENSENSSIAL_H

namespace GestureEng
{
	class CGstEnsenssial
	{
	public:
		CGstEnsenssial();
		~CGstEnsenssial();
		virtual void			GstTell(const std::vector<sAction> &r, sGesture &g){};
		virtual void			SetGestureParam(const sGestureParam *gc);
		virtual void			SetClickParam(const sGestureParamClick *gc){};
		virtual void			SetDClickParam(const sGestureParamDClick *gc){};
		virtual void			SetFlipParam(const sGestureParamFlip *gc){};
		virtual void			SetSlideBackParam(const sGestureParamSlideBack *gc){};
		virtual void			SetZARParam(const sGestureParamZoomARoatate *gc){};
		virtual void			ResetParam();
		virtual sGestureParam*				GetBasicParam();
		virtual sGestureParamClick*			GetClickParam(){ return NULL; };
		virtual sGestureParamDClick*		GetDclickParam(){ return NULL; };
		virtual	sGestureParamFlip*			GetFlipParam(){ return NULL; };
		virtual sGestureParamSlideBack*		GetSlideBackParam(){ return NULL; };
		virtual sGestureParamZoomARoatate*	GetZARParam(){ return NULL; };
		virtual void			SetFlag( const GestureEng::SHANDGST &sg, GestureEng::GSTFLAG &flag);
		virtual void			ResetFlag(SHANDGST &sg, GSTFLAG &flag);
		virtual void			CleanQue(sGesture &g);
		virtual void			DataRecieve(const std::vector<sAction> &r, sGesture &g);
		virtual bool			Equal(const GSTFLAG &a, const GSTFLAG &b);
		virtual bool			OnAction(const std::vector<sAction> &r, sGesture &s);

	protected:
		sGesture					m_sg_over;
//		GestureEng::GSTFLAG			m_gstflag;
		sGestureParam				*m_pparam;

		std::deque<PHand>					m_phands;
		std::deque<GestureEng::SHANDGST>	m_sgs;
		std::deque<GestureEng::GSTFLAG>		m_gstflags;


	private:

	};
	typedef std::shared_ptr<CGstEnsenssial> PCGstEnsenssial;

	PCGstEnsenssial			CreateCGClick();
	PCGstEnsenssial			CreateCGDClick();
	PCGstEnsenssial			CreateCGDrag();
	PCGstEnsenssial			CreateCGFlipDown();
	PCGstEnsenssial			CreateCGFlipUp();
	PCGstEnsenssial			CreateCGFlipLeft();
	PCGstEnsenssial			CreateCGFlipRight();
	PCGstEnsenssial			CreateCGSlideBackHori();
	PCGstEnsenssial			CreateCGSlideBackVert();
	PCGstEnsenssial			CreateCGLongPush();
	PCGstEnsenssial			CreateCGMoveTo();
	PCGstEnsenssial			CreateCGZooming();
	PCGstEnsenssial			CreateCGRotate();
	PCGstEnsenssial			CreateCGNone();
}
#endif
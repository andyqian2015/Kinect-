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
		virtual void			GstTell(sAction r, sGesture &g){};
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
		virtual void			SetFlag();
		virtual void			ResetFlag();
		virtual void			CleanQue(sGesture &g);
		virtual void			DataRecieve(sAction r, sGesture &g);
		virtual bool			Equal(GSTFLAG a, GSTFLAG b);
		virtual bool			OnAction(sAction r, sGesture &s);

	protected:
		GestureEng::SHANDGST		m_sg_1st, m_sg_2nd;  ////
		sGesture					m_sg_over;
		GestureEng::GSTFLAG			m_gstflag;
		sGestureParam				*m_pparam;

		CHand						*hand_1st;
		CHand						*hand_2nd;


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
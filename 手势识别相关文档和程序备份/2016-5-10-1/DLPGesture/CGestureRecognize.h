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

#ifndef GSTRCN_H
#define GSTRCN_H


namespace GestureEng
{
	class  CGestureRecognize : public IGesture
	{
	public:
		CGestureRecognize();
		void				OnAction(const std::vector<sAction> &r, std::vector<sGesture> &g);
		int					SetGestureParam(const sGestureParam *gc);
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

		GestureEng::SHANDGST			m_sg_1st, m_sg_2nd;
		sGesture						m_sg_over;
		GestureEng::GSTFLAG				m_gstflag;
		//sGestureParam			m_parameter;

		struct eGesture{
			PCGstEnsenssial		pEng;
			bool				bEnable;
		};
		std::vector<eGesture>	m_Gestures;
	};
}
#endif 
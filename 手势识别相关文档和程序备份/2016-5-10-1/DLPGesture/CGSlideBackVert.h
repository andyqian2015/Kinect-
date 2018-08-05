//GestureEngine:´¹Ö±ÇÐ»»Ä£Ê½
#pragma once

#ifndef CGSLIDEBACKVERT_H
#define CGSLIDEBACKVERT_H

#include "CGstEnsenssial.h"
#include "DLPGesture/GestureParam.h"
#include "math.h"

namespace GestureEng
{
	class CGSlideBackVert : public CGstEnsenssial
	{
	public:
		CGSlideBackVert();
		~CGSlideBackVert();
		//void			SetGestureParam(const sGestureParamSlideBack *gc);
		void						SetSlideBackParam(const sGestureParamSlideBack *gc);
		sGestureParamSlideBack*		GetSlideBackParam();
		void						ResetParam();

	private:
		void			GstTell(const std::vector<sAction> &r, sGesture &g);
		void			BackConfirm(const SHANDGST &data, sGesture &g, int id=0);
		void			GestureOut(sGesture &g);
		void			ResetCtn();

		sGesture							m_gstout;
		GSTFLAG								m_Flag_Back;
		float								m_x_max, m_y_max;
		float								m_temp_x, m_temp_y;
		float								m_x_back, m_y_back;
		sGestureParamSlideBack				*m_param;
	};
}
#endif
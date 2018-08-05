//GestureEngine:水平切换模式
#pragma once
#ifndef CGSLIDEBACKHORI_H
#define CGSLIDEBACKHORI_H

#include "CGstEnsenssial.h"
#include "DLPGesture/GestureParam.h"
#include "math.h"

namespace GestureEng
{
	class CGSlideBackHori : public CGstEnsenssial
	{
	public:
		CGSlideBackHori();
		~CGSlideBackHori();
		//void			SetGestureParam(const sGestureParamSlideBack *gc);
		void							SetSlideBackParam(const sGestureParamSlideBack *gc);
		sGestureParamSlideBack*			GetSlideBackParam();
		void							ResetParam();

	private:
		void			GstTell(sAction r, sGesture &g);
		void			BackConfirm(SHANDGST data, sGesture &g);
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
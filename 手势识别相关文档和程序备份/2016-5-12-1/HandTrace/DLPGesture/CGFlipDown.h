//GestureEngine:�·�ҳ
#pragma once

#ifndef CGFLIPDOWN_H
#define CGFLIPDOWN_H

#include "CGstEnsenssial.h"
#include "DLPGesture/GestureParam.h"
#include "math.h"

namespace GestureEng
{
	class CGFlipDown : public CGstEnsenssial
	{
	public:
		CGFlipDown();
		~CGFlipDown();
		//void					SetGestureParam(const sGestureParam *gc);
		void					SetFlipParam(const sGestureParamFlip *gc);
		sGestureParamFlip*		GetFlipParam();
		void					ResetParam();

	private:
		void			GstTell(sAction r, sGesture &g);
		bool			DistanceConfirm(SHANDGST data, sGesture &g);
		bool			SpeedConfirm(SHANDGST data, sGesture &g);
		void			ResetCtn();
		void			GestureOut(sGesture &g);

		GestureEng::GSTFLAG				m_Flag_Flip;
		sGesture						m_gstout;
		double							m_speed_x, m_speed_y;
		float							m_distance_x, m_distance_y;
		sGestureParamFlip				*m_param;
	};
}

#endif
//GestureEngine:�Ϸ�ҳ
#ifndef CGFLIPUP_H
#define CGFLIPUP_H

#include "CGstEnsenssial.h"
#include "DLPGesture/GestureParam.h"
#include "math.h"

namespace GestureEng
{
	class CGFlipUp : public CGstEnsenssial
	{
	public:
		CGFlipUp();
		~CGFlipUp();
		//void			SetGestureParam(const sGestureParamFlip *gc);
		void						SetFlipParam(const sGestureParamFlip *gc);
		sGestureParamFlip*			GetFlipParam();
		void						ResetParam();

	private:
		void			GstTell(const std::vector<sAction> &r, sGesture &g);
		bool			DistanceConfirm(const SHANDGST &data, sGesture &g);
		bool			SpeedConfirm(const SHANDGST &data, sGesture &g);
		void			ResetCtn();
		void			GestureOut(sGesture &g);

		GestureEng::GSTFLAG					m_Flag_Flip;
		sGesture							m_gstout;
		double								m_speed_x, m_speed_y;
		float								m_distance_x, m_distance_y;
		sGestureParamFlip					*m_param;
	};
}

#endif
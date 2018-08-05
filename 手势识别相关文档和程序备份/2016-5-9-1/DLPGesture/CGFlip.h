//GestureEngine:翻页 判断是否形成翻页条件并传出翻页方向
#pragma once

#include "CGstEnsenssial.h"
#include "math.h"

#ifndef CGFLIP_H
#define CGFLIP_H


class CGFlip : public CGstEnsenssial
{
public:
	CGFlip();

private:
	void			GstTell(sAction r, sGesture &g);
	bool			DistanceConfirm(inner::SHANDGST data,sGesture &g);
	bool			SpeedConfirm(inner::SHANDGST data,sGesture &g);
	void			ResetCtn();
	void			GestureOut(sGesture &g);

	inner::GSTFLAG		m_Flag_Flip;
	sGesture			m_gstout;
	double				m_speed_x,m_speed_y;
	float				m_distance_x,m_distance_y;
};

#endif
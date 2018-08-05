//GestureEngine:Ëõ·ÅºÍÐý×ª
#pragma once

#include "CGstEnsenssial.h"
#include "math.h"

#ifndef CGZOOMAROT_H
#define CGZOOMAROT_H

class CGZoomARot : public CGstEnsenssial
{
public:
	CGZoomARot();

private:
	void			GstTell(sAction r, sGesture &g);
	void			ZoomOrRot(inner::SHANDGST data1,inner::SHANDGST data2,sGesture &g);
	void			GestureOut(sGesture &g);
	void			ResetCtn();
	double			Angle(inner::SHANDGST data1,inner::SHANDGST data2);
	long double		Distance(inner::SHANDGST data1,inner::SHANDGST data2);
	Vec3<float>		CentrePoint(inner::SHANDGST data1, inner::SHANDGST data2);
	void			SetFlag();

	sGesture		m_gstout;
	inner::SHANDGST m_sav1,m_sav2;
	double			m_angle_sav,m_angle;
	long double		m_dist_sav,m_dist;
	double			m_prop_sav,m_prop;
	Vec3<float>		m_c_point_sav, m_c_point;
};
#endif
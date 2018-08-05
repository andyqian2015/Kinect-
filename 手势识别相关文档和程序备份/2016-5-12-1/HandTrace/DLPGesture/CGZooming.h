//GestureEngine:Ëõ·Å
#pragma once

#ifndef CGZOOMING_H
#define CGZOOMING_H

#include "CGstEnsenssial.h"
#include "DLPGesture/GestureParam.h"
#include "math.h"
namespace GestureEng
{
	class CGZooming : public CGstEnsenssial
	{
	public:
		CGZooming();
		~CGZooming();
		//void			SetGestureParam(const sGestureParamZoomARoatate *gc);
		void						SetZARParam(const sGestureParamZoomARoatate *gc);
		sGestureParamZoomARoatate*	GetZARParam();
		void						ResetParam();

	private:
		void						GstTell(sAction r, sGesture &g);
		void						ZoomOrRot(SHANDGST data1, SHANDGST data2, sGesture &g);
		void						GestureOut(sGesture &g);
		void						ResetCtn();
		double						Angle(SHANDGST data1, SHANDGST data2);
		long double					Distance(SHANDGST data1, SHANDGST data2);
		COORD3		                CentrePoint(SHANDGST data1, SHANDGST data2);
		void						SetFlag();

		sGesture								m_gstout;
		SHANDGST								m_sav1, m_sav2;
		double									m_angle_sav, m_angle;
		long double								m_dist_sav, m_dist;
		double									m_prop_sav, m_prop;
		COORD3								m_c_point_sav, m_c_point;
		sGestureParamZoomARoatate				*m_param;
	};
}
#endif
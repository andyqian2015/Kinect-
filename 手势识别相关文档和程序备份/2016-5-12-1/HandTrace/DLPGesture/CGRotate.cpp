#include "stdafx.h"
#include "CGRotate.h"
#include "CHand.h"

namespace GestureEng
{
	CGRotate::CGRotate() :CGstEnsenssial()
	{
		m_gstout.gtype = DualHand;
		m_gstout.pos.x = 0.0;
		m_gstout.pos.y = 0.0;
		m_gstout.t = 0;
		m_gstout.fScale = 1;
		m_gstout.fScreenRotateAngle = 0;
		m_gstout.fScreenPivot.x = 0.0;
		m_gstout.fScreenPivot.y = 0.0;
		m_c_point_sav.x = 0;
		m_c_point_sav.y = 0;
		m_c_point = m_c_point_sav;
		m_angle_sav = 0;
		m_angle = 0;
		m_prop_sav = 1;
		m_prop = 1;
		m_param = new sGestureParamZoomARoatate;
	}

	CGRotate::~CGRotate()
	{
		delete m_param;
	}

	void CGRotate::GstTell(sAction r, sGesture &g)
	{
		DataRecieve(r, g);
		if (m_gstflag.dhand_flag)
		{
			if (m_gstflag.move_flag)
			{
				if ((m_sg_1st.st)&(m_sg_2nd.st))
				{
					ZoomOrRot(m_sg_1st, m_sg_2nd, g);
				}
			}
			m_c_point_sav = CentrePoint(m_sg_1st, m_sg_2nd);
			m_angle_sav = Angle(m_sg_1st, m_sg_2nd);
			m_dist_sav = Distance(m_sg_1st, m_sg_2nd);
		}
		if (m_gstflag.over_flag | m_gstflag.click_flag)
		{
			//置手势结束标志
			m_c_point_sav.x = m_c_point_sav.y = 0;
			m_angle_sav = m_prop_sav = 0;
			ResetCtn();
			ResetFlag();


			if (hand_1st)
			   hand_1st->ResetFlag_all();
			if (hand_2nd)
			   hand_2nd->ResetFlag_all();
		}
	}

	void CGRotate::SetFlag()
	{
		if (Click == m_sg_1st.st || Click == m_sg_2nd.st)
		{
			m_gstflag.click_flag = true;
		}
		else if (Moveto == m_sg_1st.st || Moveto == m_sg_2nd.st)
		{
			m_gstflag.move_flag = true;
		}
		else if (Lpush == m_sg_1st.st || Lpush == m_sg_2nd.st)
		{
			m_gstflag.push_flag = true;
		}
		else if (m_sg_1st.overflag | m_sg_2nd.overflag)
		{
			m_gstflag.over_flag = true;
		}

	}

	void CGRotate::ZoomOrRot(SHANDGST data1, SHANDGST data2, sGesture &g)
	{
		m_angle = Angle(data1, data2) - m_angle_sav;
		m_dist = Distance(data1, data2);
		m_prop = m_dist / m_dist_sav;
		if (m_angle != 0)
		{
			m_c_point = CentrePoint(data1, data2);
			m_gstout.gtype = GRotate;
			m_gstout.fScreenRotateAngle = m_angle*(1+m_param->angle_rotate);
			m_gstout.pos = m_c_point;
			m_gstout.fScreenPivot = m_c_point_sav;
			if (data1.t < data2.t)
			{
				m_gstout.t = data2.t;
			}
			else
			{
				m_gstout.t = data1.t;
			}
			GestureOut(g);
		}

	}

	void CGRotate::SetZARParam(const sGestureParamZoomARoatate *gc)
	{
		m_param->angle_rotate = gc->angle_rotate;
		m_param->proportion_zoom = gc->proportion_zoom;
		m_pparam->radius_move = gc->radius_move;
		m_pparam->time_longpush = gc->time_longpush;
		m_pparam->time_max = gc->time_max;
	}

	void CGRotate::ResetParam()
	{
		m_pparam->radius_move = 10;
		m_pparam->time_longpush = 1000;
		m_pparam->time_max = 120000;
		m_param->angle_rotate = 0;
		m_param->proportion_zoom = 0;
	}

	sGestureParamZoomARoatate*	CGRotate::GetZARParam()
	{
		return m_param;
	}

	double CGRotate::Angle(SHANDGST data1, SHANDGST data2)
	{
		double a;
		a = std::atan2((data2.pos_fin.y - data1.pos_fin.y), (data2.pos_fin.x - data1.pos_fin.x));
		return a;
	}

	long double CGRotate::Distance(SHANDGST data1, SHANDGST data2)
	{
		long double a;
		a = std::sqrtl((data2.pos_fin.y - data1.pos_fin.y)*(data2.pos_fin.y - data1.pos_fin.y) + (data2.pos_fin.x - data1.pos_fin.x)*(data2.pos_fin.x - data1.pos_fin.x));
		return a;
	}

	COORD3 CGRotate::CentrePoint(SHANDGST data1, SHANDGST data2)
	{
		COORD3 p;
		p.x = (data1.pos_fin.x + data2.pos_fin.x);
		p.y = (data1.pos_fin.y + data2.pos_fin.y);
		return p;
	}

	void CGRotate::ResetCtn()
	{
		m_gstout.gtype = DualHand;
// 		m_gstout.pos.x = 0.0;
// 		m_gstout.pos.y = 0.0;
// 		m_gstout.t = 0;
// 		m_gstout.fScale = 1;
// 		m_gstout.fScreenRotateAngle = 0;
// 		m_gstout.fScreenPivot.x = 0.0;
// 		m_gstout.fScreenPivot.y = 0.0;
		m_c_point.x = 0;
		m_c_point.y = 0;
		m_angle = 0;
		m_prop = 0;
	}

	void CGRotate::GestureOut(sGesture &g)
	{
		g = m_gstout;
	}
}
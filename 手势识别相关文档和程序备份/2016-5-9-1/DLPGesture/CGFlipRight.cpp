#include "stdafx.h"
#include "CGFlipRight.h"

namespace GestureEng
{
	CGFlipRight::CGFlipRight() :CGstEnsenssial()
	{
		m_gstout.gtype = DualHand;
		m_gstout.pos.x = 0.0;
		m_gstout.pos.y = 0.0;
		m_gstout.t = 0;
		m_gstout.fScale = 1;
		m_gstout.fScreenRotateAngle = 0;
		m_gstout.fScreenPivot.x = 0.0;
		m_gstout.fScreenPivot.y = 0.0;
		m_speed_x = 0;
		m_speed_y = 0;
		m_distance_x = 0;
		m_distance_y = 0;
		m_Flag_Flip.dhand_flag = false;
		m_Flag_Flip.click_flag = false;
		m_Flag_Flip.move_flag = true;
		m_Flag_Flip.push_flag = false;
		m_Flag_Flip.over_flag = true;
		m_param = new sGestureParamFlip;
	}

	CGFlipRight::~CGFlipRight()
	{
		delete m_param;
	}

	void CGFlipRight::GstTell(const std::vector<sAction> &r, sGesture &g)
	{
		DataRecieve(r, g);
		for (int i = 0; i != r.size(); ++i)
		{
			int id = r[i].ID;
			if (!m_gstflags[0].dhand_flag)
			{
				if (Equal(m_Flag_Flip, m_gstflags[id]))
				{
					if (DistanceConfirm(m_sgs[id], g))
					{

					}
					else
					{
						SpeedConfirm(m_sgs[id], g);
					}
				}
			}
			if (m_gstflags[id].over_flag || m_gstflags[id].click_flag)
			{
				ResetFlag(m_sgs[id], m_gstflags[id]);
			}
		}
	}

	bool CGFlipRight::DistanceConfirm(const SHANDGST &data, sGesture &g)
	{
		m_distance_x = data.pos_fin.x - data.pos_ori.x;
		m_distance_y = data.pos_fin.y - data.pos_ori.y;
		if ((std::abs(m_distance_x) > m_param->distance_flip) || ((std::abs(m_distance_y)) > m_param->distance_flip))
		{
			if (std::abs(m_distance_x) > std::abs(m_distance_y))
			{
				if (m_distance_x > 0)
				{
					m_gstout.gtype = GFlipRight;
					m_gstout.t = data.t;
					m_gstout.pos = data.pos_fin;
					GestureOut(g);
					ResetCtn();
				}
			}
			m_distance_x = m_distance_y = 0;
			return true;
		}
		else
		{
			return false;
		}
	}

	bool CGFlipRight::SpeedConfirm(const SHANDGST &data, sGesture &g)
	{
		m_speed_x = (m_distance_x) / (data.t - data.t_ori);
		m_speed_y = (m_distance_y) / (data.t - data.t_ori);
		if ((std::abs(m_speed_x) > m_param->speed_flip) || ((std::abs(m_speed_y)) > m_param->speed_flip))
		{
			if (std::abs(m_speed_x) > std::abs(m_speed_y))
			{
				if (m_speed_x > 0)
				{
					m_gstout.gtype = GFlipRight;
					m_gstout.t = data.t;
					m_gstout.pos = data.pos_fin;
					GestureOut(g);
					ResetCtn();
				}
			}
			m_speed_x = m_speed_y = 0;
			return true;
		}
		else
		{
			return false;
		}
	}

	void CGFlipRight::SetFlipParam(const sGestureParamFlip *gc)
	{
		m_param->distance_flip = gc->distance_flip;
		m_param->speed_flip = gc->speed_flip;
		m_pparam->radius_move = gc->radius_move;
		m_pparam->time_longpush = gc->time_longpush;
		m_pparam->time_max = gc->time_max;
	}

	void CGFlipRight::ResetParam()
	{
		m_pparam->radius_move = 10;
		m_pparam->time_longpush = 1000;
		m_pparam->time_max = 120000;
		m_param->distance_flip = 600;
		m_param->speed_flip = 1.0;
	}

	sGestureParamFlip*	CGFlipRight::GetFlipParam()
	{
		return m_param;
	}

	void CGFlipRight::ResetCtn()
	{
		m_gstout.gtype = DualHand;
// 		m_gstout.pos.x = 0.0;
// 		m_gstout.pos.y = 0.0;
// 		m_gstout.t = 0;
// 		m_gstout.fScale = 1;
// 		m_gstout.fScreenRotateAngle = 0;
// 		m_gstout.fScreenPivot.x = 0.0;
// 		m_gstout.fScreenPivot.y = 0.0;
	}
	void CGFlipRight::GestureOut(sGesture &g)
	{
		g = m_gstout;
	}
}
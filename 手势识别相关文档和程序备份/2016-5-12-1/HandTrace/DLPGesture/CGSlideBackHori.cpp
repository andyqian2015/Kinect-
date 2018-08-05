#include "stdafx.h"
#include "CGSlideBackHori.h"

namespace GestureEng
{
	CGSlideBackHori::CGSlideBackHori() :CGstEnsenssial()
	{
		m_gstout.gtype = DualHand;
		m_gstout.pos.x = 0.0;
		m_gstout.pos.y = 0.0;
		m_gstout.t = 0;
		m_gstout.fScale = 1;
		m_gstout.fScreenRotateAngle = 0;
		m_gstout.fScreenPivot.x = 0.0;
		m_gstout.fScreenPivot.y = 0.0;
		m_x_max = 0;
		m_y_max = 0;
		m_x_back = 0;
		m_y_back = 0;
		m_Flag_Back.dhand_flag = false;
		m_Flag_Back.click_flag = false;
		m_Flag_Back.move_flag = true;
		m_Flag_Back.push_flag = false;
		m_Flag_Back.over_flag = false;
		m_param = new sGestureParamSlideBack;
	}

	CGSlideBackHori::~CGSlideBackHori()
	{
		delete m_param;
	}

	void CGSlideBackHori::GstTell(sAction r, sGesture &g)
	{
		DataRecieve(r, g);
		if (Equal(m_Flag_Back, m_gstflag))
		{
			BackConfirm(m_sg_1st, g);
		}
		if (m_gstflag.over_flag | m_gstflag.click_flag)
		{

			m_x_back = m_x_max = m_temp_x = 0;
			m_y_back = m_y_max = m_temp_y = 0;
			ResetFlag();
		}
	}

	void CGSlideBackHori::BackConfirm(SHANDGST data, sGesture &g)
	{
		m_temp_x = (data.pos_fin.x - data.pos_ori.x);
		m_temp_y = (data.pos_fin.y - data.pos_ori.y);
		if (std::abs(m_x_max) <= (std::abs(m_temp_x)))
		{
			m_x_max = m_temp_x;
		}
		else
		{
			m_x_back = std::abs(m_x_max - m_temp_x);
		}
		if (std::abs(m_y_max)<=(std::abs(m_temp_y)))
		{
			m_y_max = m_temp_y;
		}
		else
		{
			m_y_back = std::abs(m_y_max - m_temp_y);
		}
		if ((std::abs(m_x_max)> m_param->distance_slide) | (std::abs(m_y_max) > m_param->distance_slide))
		{
			bool backenough_x = (m_x_back > m_param->distance_back) | (m_x_back > (m_param->distance_slide*m_param->proportion_back));
			bool backenough_y = (m_y_back > m_param->distance_back) | (m_y_back > (m_param->distance_slide*m_param->proportion_back));
			if (backenough_x | backenough_y)
			{
				if (m_x_back > m_y_back)
				{
					if (std::abs(m_y_max) < m_param->distance_slide)
					{
						m_gstout.gtype = GSwithHori;
						m_gstout.t = data.t;
						m_gstout.pos = data.pos_fin;
						GestureOut(g);
						//置手势结束标志
					}
					// 					hand_1st->ResetFlag_all();
					// 					ResetCtn();
					// 					m_x_back = m_x_max = m_temp_x = 0;
					// 					m_y_back = m_y_max = m_temp_y = 0;
				}
			ResetFlag();

			if (hand_1st)
			    hand_1st->ResetFlag_all();
			ResetCtn();
			m_x_back = m_x_max = m_temp_x = 0;
			m_y_back = m_y_max = m_temp_y = 0;

			}
		}
	}

	void CGSlideBackHori::SetSlideBackParam(const sGestureParamSlideBack *gc)
	{
		m_param->distance_back = gc->distance_back;
		m_param->distance_slide = gc->distance_slide;
		m_param->proportion_back = gc->proportion_back;
		m_pparam->radius_move = gc->radius_move;
		m_pparam->time_longpush = gc->time_longpush;
		m_pparam->time_max = gc->time_max;
	}

	void CGSlideBackHori::ResetParam()
	{
		m_pparam->radius_move = 10;
		m_pparam->time_longpush = 1000;
		m_pparam->time_max = 120000;
		m_param->distance_back = 100;
		m_param->proportion_back = 0.5;
		m_param->distance_slide = 500;
	}

	sGestureParamSlideBack*	CGSlideBackHori::GetSlideBackParam()
	{
		return m_param;
	}

	void CGSlideBackHori::ResetCtn()
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

	void CGSlideBackHori::GestureOut(sGesture &g)
	{
		g = m_gstout;
	}
}
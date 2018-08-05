#include "stdafx.h"
#include "CGSlideBack.h"

CGSlideBack::CGSlideBack() :CGstEnsenssial()
{
	m_gstout.gtype = GNone;
	m_gstout.pos.x = 0.0;
	m_gstout.pos.y = 0.0;
	m_gstout.t = 0;
	m_gstout.fScale = 1;
	m_gstout.fScreenRotateAngle = 0;
	m_gstout.fScreenPivot.x = 0.0;
	m_gstout.fScreenPivot.y = 0.0;
	m_x_max=0;
	m_y_max=0;
	m_x_back=0;
	m_y_back=0;
	m_Flag_Back.dhand_flag=false;
	m_Flag_Back.click_flag = false;
	m_Flag_Back.move_flag = true;
	m_Flag_Back.push_flag = false;
	m_Flag_Back.over_flag = false;
}

void CGSlideBack::GstTell(sAction r, sGesture &g)
{
	DataRecieve(r, g);
	if (Equal(m_Flag_Back,m_gstflag))
	{
		BackConfirm(m_sg_1st,g);
	}
	if (m_gstflag.over_flag)
	{
		ResetFlag();
	}
}

void CGSlideBack::BackConfirm(inner::SHANDGST data,sGesture &g)
{
	float distance = 600;
	m_temp_x=(data.pos_fin.x-data.pos_ori.x);
	m_temp_y=(data.pos_fin.y-data.pos_ori.y);
	if (std::abs(m_x_max)<(std::abs(m_temp_x)))
	{
		m_x_max=m_temp_x;
	}
	else
	{
		m_x_back=std::abs(m_x_max-m_temp_x);
	}
	if (std::abs(m_y_max)<(std::abs(m_temp_y)))
	{
		m_y_max=m_temp_y;
	}
	else
	{
		m_y_back=std::abs(m_y_max-m_temp_y);
	}
	
	if ((std::abs(m_x_max)>distance)|(std::abs(m_y_max)>distance))
	{
		float back_confirm=100;
		if ((m_x_back>back_confirm)|(m_y_back>back_confirm))
		{
			if (m_x_back>m_y_back)
			{
				if (m_y_max<distance)
				{
					m_gstout.gtype=GSwithHori;
					m_gstout.t=data.t;
					m_gstout.pos=data.pos_fin;
					GestureOut(g);
					//置手势结束标志
					ResetFlag();
					hand_1st->ResetFlag_all();
					ResetCtn();
					m_x_back = m_x_max = m_temp_x = 0;
					m_y_back = m_y_max = m_temp_y = 0;
				}
			}
			else
			{
				if (std::abs(m_x_max)<distance)
				{
					m_gstout.gtype=GSwithVert;
					m_gstout.t=data.t;
					m_gstout.pos=data.pos_fin;
					GestureOut(g);
					//置手势结束标志
					ResetFlag();
					hand_1st->ResetFlag_all();
					ResetCtn();
					m_x_back = m_x_max = m_temp_x = 0;
					m_y_back = m_y_max = m_temp_y = 0;
				}
			}
		}
	}
}

void CGSlideBack::ResetCtn()
{
	m_gstout.gtype = GNone;
	m_gstout.pos.x = 0.0;
	m_gstout.pos.y = 0.0;
	m_gstout.t = 0;
	m_gstout.fScale = 1;
	m_gstout.fScreenRotateAngle = 0;
	m_gstout.fScreenPivot.x = 0.0;
	m_gstout.fScreenPivot.y = 0.0;
}

void CGSlideBack::GestureOut(sGesture &g)
{
	g = m_gstout;
}
#include "stdafx.h"
#include "CGFlip.h"

CGFlip::CGFlip() :CGstEnsenssial()
{
	m_gstout.gtype = GNone;
	m_gstout.pos.x = 0.0;
	m_gstout.pos.y = 0.0;
	m_gstout.t = 0;
	m_gstout.fScale = 1;
	m_gstout.fScreenRotateAngle = 0;
	m_gstout.fScreenPivot.x = 0.0;
	m_gstout.fScreenPivot.y = 0.0;
	m_speed_x=0;
	m_speed_y=0;
	m_distance_x=0;
	m_distance_y=0;
	m_Flag_Flip.dhand_flag=false;
	m_Flag_Flip.click_flag = false;
	m_Flag_Flip.move_flag = true;
	m_Flag_Flip.push_flag = false;
	m_Flag_Flip.over_flag = true;
}

void CGFlip::GstTell(sAction r, sGesture &g)
{
	DataRecieve(r, g);
	if (Equal(m_Flag_Flip,m_gstflag))
	{
		if(DistanceConfirm(m_sg_1st,g))
		{

		}
		else
		{
			SpeedConfirm(m_sg_1st,g);
		}
	}
	if (m_gstflag.over_flag)
	{
		ResetFlag();
	}
}

bool CGFlip::DistanceConfirm(inner::SHANDGST data,sGesture &g)
{
	float distance=600;
	m_distance_x=data.pos_fin.x-data.pos_ori.x;
	m_distance_y=data.pos_fin.y-data.pos_ori.y;
	if ((std::abs(m_distance_x)>distance)|((std::abs(m_distance_y))>distance))
	{
		if (std::abs(m_distance_x)>std::abs(m_distance_y))
		{
			if (m_distance_x>0)
			{
				m_gstout.gtype=GFlipRight;
				m_gstout.t=data.t;
				m_gstout.pos=data.pos_fin;
				GestureOut(g);
				ResetCtn();
				m_distance_x = m_distance_y = 0;
			}
			else
			{
				m_gstout.gtype=GFlipLeft;
				m_gstout.t=data.t;
				m_gstout.pos=data.pos_fin;
				GestureOut(g);
				ResetCtn();
				m_distance_x = m_distance_y = 0;
			}
		}
		else
		{
			if (m_distance_y>0)
			{
				m_gstout.gtype=GFlipDown;
				m_gstout.t=data.t;
				m_gstout.pos=data.pos_fin;
				GestureOut(g);
				ResetCtn();
				m_distance_x = m_distance_y = 0;
			}
			else
			{
				m_gstout.gtype=GFlipUp;
				m_gstout.t=data.t;
				m_gstout.pos=data.pos_fin;
				GestureOut(g);
				ResetCtn();
				m_distance_x = m_distance_y = 0;
			}
		}
		return true;
	}
	else
	{
		return false;
	}
}

bool CGFlip::SpeedConfirm(inner::SHANDGST data,sGesture &g)
{
	double v=1.0;
	m_speed_x=(m_distance_x)/(data.t-data.t_ori);
	m_speed_y=(m_distance_y)/(data.t-data.t_ori);
	if ((std::abs(m_speed_x)>v)|((std::abs(m_speed_y))>v))
	{
		if (std::abs(m_speed_x)>std::abs(m_speed_y))
		{
			if (m_speed_x>0)
			{
				m_gstout.gtype=GFlipRight;
				m_gstout.t=data.t;
				m_gstout.pos=data.pos_fin;
				GestureOut(g);
				ResetCtn();
				m_speed_x = m_speed_y = 0;
			}
			else
			{
				m_gstout.gtype=GFlipLeft;
				m_gstout.t=data.t;
				m_gstout.pos=data.pos_fin;
				GestureOut(g);
				ResetCtn();
				m_speed_x = m_speed_y = 0;
			}
		}
		else
		{
			if (m_speed_y>0)
			{
				m_gstout.gtype=GFlipDown;
				m_gstout.t=data.t;
				m_gstout.pos=data.pos_fin;
				GestureOut(g);
				ResetCtn();
				m_speed_x = m_speed_y = 0;
			}
			else
			{
				m_gstout.gtype=GFlipUp;
				m_gstout.t=data.t;
				m_gstout.pos=data.pos_fin;
				GestureOut(g);
				ResetCtn();
				m_speed_x = m_speed_y = 0;
			}
		}
		return true;
	}
	else
	{
		return false;
	}
}

void CGFlip::ResetCtn()
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
void CGFlip::GestureOut(sGesture &g)
{
	g = m_gstout;
}
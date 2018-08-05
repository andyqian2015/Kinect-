#include "stdafx.h"
#include "CGDoubleClick.h"

namespace GestureEng
{
	CGDoubleClick::CGDoubleClick() :CGstEnsenssial()
	{
		m_gstout.gtype = DualHand;
		m_gstout.pos.x = 0.0;
		m_gstout.pos.y = 0.0;
		m_gstout.t = 0;
		m_gstout.fScale = 1;
		m_gstout.fScreenRotateAngle = 0;
		m_gstout.fScreenPivot.x = 0.0;
		m_gstout.fScreenPivot.y = 0.0;
		m_c_flag = false;
		m_sav.overflag = false;
		m_sav.pos_ori.x = 0;
		m_sav.pos_ori.y = 0;
		m_sav.pos_fin.x = 0;
		m_sav.pos_fin.y = 0;
		m_sav.st = GestureEng::Neither;
		m_sav.t = 0;
		m_sav.t_ori = 0;
		m_param = new sGestureParamDClick;
	}

	CGDoubleClick::~CGDoubleClick()
	{
		delete m_param;
	}

	void CGDoubleClick::GstTell(const std::vector<sAction> &r, sGesture &g)
	{
		DataRecieve(r, g);
		for (int i = 0; i != r.size();++i)
		{
			int id = r[i].ID;
			if (!m_gstflags[0].dhand_flag)
			{
				if (m_gstflags[id].click_flag)
				{
					if (m_c_flag&&ClickConfirm(m_sgs[id], m_gstflags[id]))
					{
						m_gstout.gtype = GDoubleClick;
						m_gstout.t = m_sgs[id].t;
						m_gstout.pos = m_sgs[id].pos_ori;
						GestureOut(g);
						ResetCtn();
					}
					else
					{
						m_c_flag = true;
						m_sav = m_sgs[id];
					}
					/*else//若与底层模块属同一线程则难实现
					{
					gstout.gtype=GSingleClick;
					gstout.t=data.t;
					gstout.pos=data.pos_ori;
					}*/
				}
			}
			ResetFlag(m_sgs[id], m_gstflags[id]);
		}
	}

	bool CGDoubleClick::ClickConfirm(const SHANDGST &data, const GSTFLAG &flag)
	{
		if (flag.click_flag)
		{
			if (((data.t - m_sav.t) < m_param->time_dclick)&&(std::sqrtl((data.pos_fin.y - m_sav.pos_fin.y)*(data.pos_fin.y - m_sav.pos_fin.y) + (data.pos_fin.x - m_sav.pos_fin.x)*(data.pos_fin.x - m_sav.pos_fin.x)) < m_param->radius_dclick))
			{
				m_c_flag = false;
				return true;
			}
			else
			{
				m_c_flag = false;
				return false;
			}
		}
		else
		{
			m_c_flag = false;
			return false;
		}
	}

	void CGDoubleClick::SetDClickParam(const sGestureParamDClick *gc)
	{
		m_param->radius_dclick = gc->radius_dclick;
		m_param->time_dclick = gc->time_dclick;
		m_pparam->radius_move = gc->radius_move;
		m_pparam->time_longpush = gc->time_longpush;
		m_pparam->time_max = gc->time_max;
	}

	void CGDoubleClick::ResetParam()
	{
		m_pparam->radius_move = 10;
		m_pparam->time_longpush = 1000;
		m_pparam->time_max = 120000;
		m_param->radius_dclick = 15;
		m_param->time_dclick = 1000;
	}

	sGestureParamDClick*	CGDoubleClick::GetDclickParam()
	{
		return m_param;
	}

	void CGDoubleClick::ResetCtn()
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

	void CGDoubleClick::GestureOut(sGesture &g)
	{
		g = m_gstout;
	}
}
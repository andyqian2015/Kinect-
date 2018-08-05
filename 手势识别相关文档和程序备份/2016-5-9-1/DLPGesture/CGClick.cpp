#include "stdafx.h"
#include "CGClick.h"

namespace GestureEng
{
	CGClick::CGClick() : CGstEnsenssial()
	{
		m_gstout.gtype = DualHand;
		m_gstout.pos.x = 0.0;
		m_gstout.pos.y = 0.0;
		m_gstout.t = 0;
		m_gstout.fScale = 1;
		m_gstout.fScreenRotateAngle = 0;
		m_gstout.fScreenPivot.x = 0.0;
		m_gstout.fScreenPivot.y = 0.0;
		m_param = new sGestureParamClick;
	}

	CGClick::~CGClick()
	{
		delete m_param;
	}

	void CGClick::GstTell(const std::vector<sAction> &r, sGesture &g)
	{
		DataRecieve(r, g);
		for (int i = 0; i != r.size();++i)
		{
			int id = r[i].ID;
			if (!m_gstflags[0].dhand_flag)
			{
				if (m_gstflags[id].click_flag)
				{
					m_gstout.gtype = GClick;
					m_gstout.t = m_sgs[id].t;
					m_gstout.pos = m_sgs[id].pos_ori;
					GestureOut(g);
					ResetCtn();
				}
			}
			ResetFlag(m_sgs[id], m_gstflags[id]);
		}
	}


	void CGClick::SetClickParam(const sGestureParamClick *gc)
	{
		m_pparam->time_max = gc->time_max;
		m_pparam->radius_move = gc->radius_move;
		m_pparam->time_longpush = gc->time_longpush;
		m_param->radius_click = gc->radius_click;
	}

	void CGClick::ResetParam()
	{
		m_pparam->radius_move = 10;
		m_pparam->time_longpush = 1000;
		m_pparam->time_max = 120000;
		m_param->radius_click = 10;
	}

	sGestureParamClick* CGClick::GetClickParam()
	{
		return m_param;
	}

	void CGClick::GestureOut(sGesture &g)
	{
		g = m_gstout;
	}

	void CGClick::ResetCtn()
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
}
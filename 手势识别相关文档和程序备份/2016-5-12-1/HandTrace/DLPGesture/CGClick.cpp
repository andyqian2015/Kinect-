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

	void CGClick::GstTell(sAction r, sGesture &g)
	{
		DataRecieve(r, g);
		if (!m_gstflag.dhand_flag)
		{
			if (m_gstflag.click_flag)
			{
				m_gstout.gtype = GClick;
				m_gstout.t = m_sg_1st.t;
				m_gstout.pos = m_sg_1st.pos_ori;
				GestureOut(g);
				ResetCtn();
			}
		}
		ResetFlag();
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
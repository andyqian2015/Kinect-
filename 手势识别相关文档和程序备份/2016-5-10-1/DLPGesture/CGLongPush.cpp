#include "stdafx.h"
#include "CGLongPush.h"

namespace GestureEng
{
	CGLongPush::CGLongPush() :CGstEnsenssial()
	{
		m_gstout.gtype = DualHand;
		m_gstout.pos.x = 0.0;
		m_gstout.pos.y = 0.0;
		m_gstout.t = 0;
		m_gstout.fScale = 1;
		m_gstout.fScreenRotateAngle = 0;
		m_gstout.fScreenPivot.x = 0.0;
		m_gstout.fScreenPivot.y = 0.0;
		m_Flag_LPush.dhand_flag = false;
		m_Flag_LPush.click_flag = false;
		m_Flag_LPush.push_flag = true;
		m_Flag_LPush.move_flag = false;
		m_Flag_LPush.over_flag = false;
	}

	void CGLongPush::GstTell(const std::vector<sAction> &r, sGesture &g)
	{
		DataRecieve(r, g);
		for (int i = 0; i != r.size(); ++i)
		{
			int id = r[i].ID;
			if (!m_gstflags[0].dhand_flag)
			{
				if (Equal(m_Flag_LPush, m_gstflags[id]))
				{
					m_gstout.gtype = GLongPush;
					m_gstout.t = m_sgs[id].t;
					m_gstout.pos = m_sgs[id].pos_ori;
					GestureOut(g);
					ResetCtn();
				}
			}
			if (m_gstflags[id].over_flag || m_gstflags[id].click_flag)
			{
				//置手势结束标志
				ResetFlag(m_sgs[id], m_gstflags[id]);
			}
		}
	}

	void CGLongPush::ResetCtn()
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

	void CGLongPush::GestureOut(sGesture &g)
	{
		g = m_gstout;
	}
}
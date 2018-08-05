#include "stdafx.h"
#include "CGMoveTo.h"

namespace GestureEng
{
	CGMoveTo::CGMoveTo() :CGstEnsenssial()
	{
		m_gstout.gtype = DualHand;
		m_gstout.pos.x = 0.0;
		m_gstout.pos.y = 0.0;
		m_gstout.t = 0;
		m_gstout.fScale = 1;
		m_gstout.fScreenRotateAngle = 0;
		m_gstout.fScreenPivot.x = 0.0;
		m_gstout.fScreenPivot.y = 0.0;
	}

	void CGMoveTo::GstTell(const std::vector<sAction> &r, sGesture &g)
	{
		DataRecieve(r, g);
		for (int i = 0; i != r.size(); ++i)
		{
			int id = r[i].ID;
			if (!m_gstflags[id].dhand_flag)
			{
				if (m_gstflags[id].move_flag)
				{
					m_gstout.gtype = GMoveTo;
					m_gstout.t = m_sgs[id].t;
					m_gstout.pos = m_sgs[id].pos_fin;
					GestureOut(g);
					ResetCtn();
				}
				if (m_gstflags[id].over_flag || m_gstflags[id].click_flag)
				{
					//置手势结束标志
					ResetFlag(m_sgs[id], m_gstflags[id]);
				}
			}
		}
	}

	void CGMoveTo::ResetCtn()
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

	void CGMoveTo::GestureOut(sGesture &g)
	{
		g = m_gstout;
	}
}
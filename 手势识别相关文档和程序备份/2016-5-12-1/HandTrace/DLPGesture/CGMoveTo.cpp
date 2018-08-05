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

	void CGMoveTo::GstTell(sAction r, sGesture &g)
	{
		DataRecieve(r, g);
		if (!m_gstflag.dhand_flag)
		{
			if (m_gstflag.move_flag)
			{
				m_gstout.gtype = GMoveTo;
				m_gstout.t = m_sg_1st.t;
				m_gstout.pos = m_sg_1st.pos_fin;
				GestureOut(g);
				ResetCtn();
			}
			if (m_gstflag.over_flag | m_gstflag.click_flag)
			{
				//置手势结束标志
				ResetFlag();
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
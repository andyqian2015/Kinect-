#include "stdafx.h"
#include "CGNone.h"

namespace GestureEng
{
	CGNone::CGNone() :CGstEnsenssial()
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

	void CGNone::GstTell(sAction r, sGesture &g)
	{
		DataRecieve(r, g);
		if (Up==r.st)
		{
			m_gstout.gtype = GNone;
			m_gstout.t = m_sg_1st.t;
			m_gstout.pos = m_sg_1st.pos_fin;
			GestureOut(g);
			ResetCtn();
		}
		//if (m_gstflag.over_flag | m_gstflag.click_flag)
			ResetFlag();
	}

	void CGNone::ResetCtn()
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

	void CGNone::GestureOut(sGesture &g)
	{
		g = m_gstout;
	}
}
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

	void CGLongPush::GstTell(sAction r, sGesture &g)
	{
		DataRecieve(r, g);
		if (Equal(m_Flag_LPush, m_gstflag))
		{
			m_gstout.gtype = GLongPush;
			m_gstout.t = m_sg_1st.t;
			m_gstout.pos = m_sg_1st.pos_ori;
			GestureOut(g);
			ResetCtn();
		}
		if (m_gstflag.over_flag | m_gstflag.click_flag)
		{
			//置手势结束标志
			ResetFlag();
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
#include "stdafx.h"
#include "CGDrag.h"

namespace GestureEng
{
	CGDrag::CGDrag() :CGstEnsenssial()
	{
		m_gstout.gtype = DualHand;
		m_gstout.pos.x = 0.0;
		m_gstout.pos.y = 0.0;
		m_gstout.t = 0;
		m_gstout.fScale = 1;
		m_gstout.fScreenRotateAngle = 0;
		m_gstout.fScreenPivot.x = 0.0;
		m_gstout.fScreenPivot.y = 0.0;
		m_Flag_Drag.dhand_flag = false;
		m_Flag_Drag.click_flag = false;
		m_Flag_Drag.move_flag = true;
		m_Flag_Drag.push_flag = true;
		m_Flag_Drag.over_flag = false;
	}

	void CGDrag::GstTell(sAction r, sGesture &g)
	{
		DataRecieve(r, g);
		if (Equal(m_Flag_Drag, m_gstflag))
		{
			m_gstout.gtype = GDrag;
			m_gstout.t = m_sg_1st.t;
			m_gstout.pos.x= m_sg_1st.pos_fin.x;
			m_gstout.pos.y = m_sg_1st.pos_fin.y;
			m_gstout.pos.z = m_sg_1st.pos_fin.z;
			GestureOut(g);
			ResetCtn();
		}
		if (m_gstflag.over_flag|m_gstflag.click_flag)
		{
// 			m_gstout.gtype = GNone;
// 			GestureOut(g);
// 			ResetCtn();
			//置手势结束标志

			ResetFlag();
		}
	}

	void CGDrag::ResetCtn()
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

	void CGDrag::GestureOut(sGesture &g)
	{
		g = m_gstout;
	}
}
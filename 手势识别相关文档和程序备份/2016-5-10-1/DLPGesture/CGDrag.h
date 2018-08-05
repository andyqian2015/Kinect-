//GestureEngine:ÍÏ¶¯
#pragma once

#include "CGstEnsenssial.h"

#ifndef CGDRAG_H
#define CGDRAG_H

namespace GestureEng
{
	class CGDrag : public CGstEnsenssial
	{
	public:
		CGDrag();

	private:
		void			GstTell(const std::vector<sAction> &r, sGesture &g);
		void			GestureOut(sGesture &g);
		void			ResetCtn();

		sGesture				m_gstout;
		GestureEng::GSTFLAG		m_Flag_Drag;
	};
}
#endif
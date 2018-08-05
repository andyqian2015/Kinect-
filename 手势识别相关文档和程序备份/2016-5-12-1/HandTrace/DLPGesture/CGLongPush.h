//GestureEngine:³¤°´
#pragma once

#include "CGstEnsenssial.h"

#ifndef CGLONGPUSH_H
#define CGLONGPUSH_H
namespace GestureEng
{
	class CGLongPush : public CGstEnsenssial
	{
	public:
		CGLongPush();

	private:
		void			GstTell(sAction r, sGesture &g);
		void			GestureOut(sGesture &g);
		void			ResetCtn();

		sGesture				m_gstout;
		GestureEng::GSTFLAG		m_Flag_LPush;
	};
}
#endif
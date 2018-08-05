//GestureEngine:ÒÆ¶¯
#pragma once

#include "CGstEnsenssial.h"

#ifndef CGMOVETO_H
#define CGMOVETO_H

namespace GestureEng
{
	class CGMoveTo : public CGstEnsenssial
	{
	public:
		CGMoveTo();

	private:
		void			GstTell(sAction r, sGesture &g);
		void			GestureOut(sGesture &g);
		void			ResetCtn();

		sGesture		m_gstout;
	};
}
#endif
//手势engine:手势结束
#pragma once

#include "CGstEnsenssial.h"

#ifndef CGNONE_H
#define CGNONE_H
namespace GestureEng
{
	class CGNone : public CGstEnsenssial
	{
	public:
		CGNone();
	private:
		void			GstTell(sAction r, sGesture &g);
		void			GestureOut(sGesture &g);
		void			ResetCtn();

		sGesture		m_gstout;
	};
}

#endif
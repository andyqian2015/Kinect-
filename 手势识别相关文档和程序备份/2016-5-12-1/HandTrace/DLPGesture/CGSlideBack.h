//GestureEngine:»Ø»¬
#pragma once

#include "CGstEnsenssial.h"
#include "math.h"

#ifndef CGSLIDEBACK_H
#define CGSLIDEBACK_H

class CGSlideBack : public CGstEnsenssial
{
public:
	CGSlideBack();

private:
	void			GstTell(sAction r, sGesture &g);
	void			BackConfirm(inner::SHANDGST data,sGesture &g);
	void			GestureOut(sGesture &g);
	void			ResetCtn();

	sGesture		m_gstout;
	inner::GSTFLAG	m_Flag_Back;
	float			m_x_max,m_y_max;
	float			m_temp_x,m_temp_y;
	float			m_x_back,m_y_back;
};
#endif
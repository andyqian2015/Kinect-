//GestureEngine:µã»÷
#pragma once

#include "CGstEnsenssial.h"
#include "DLPGesture/GestureParam.h"

#ifndef CGCLICK_H
#define CGCLICK_H

namespace GestureEng
{
	class CGClick : public CGstEnsenssial
	{
	public:
		CGClick();
		~CGClick();
		//void						SetGestureParam(const sGestureParamClick *gc);
		void						SetClickParam(const sGestureParamClick *gc);
		sGestureParamClick*			GetClickParam();
		void						ResetParam();

	private:
		void			GstTell(sAction r, sGesture &g);
		void			GestureOut(sGesture &g);
		void			ResetCtn();

		sGesture						m_gstout;
		sGestureParamClick				*m_param;
	};
}
#endif
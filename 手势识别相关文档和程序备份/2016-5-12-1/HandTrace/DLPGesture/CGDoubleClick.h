//GestureEngine:˫��  �������������ͷ����޷��ж�
#pragma once

#include "CGstEnsenssial.h"
#include "DLPGesture/GestureParam.h"

#ifndef CGDOUBLECLICK_H
#define CGDOUBLECLICK_H

namespace GestureEng
{
	class CGDoubleClick : public CGstEnsenssial
	{
	public:
		CGDoubleClick();
		~CGDoubleClick();
		//void			SetGestureParam(const sGestureParamDClick *gc);
		void						SetDClickParam(const sGestureParamDClick *gc);
		sGestureParamDClick*		GetDclickParam();
		void						ResetParam();

	private:
		void			GstTell(sAction r, sGesture &g);
		bool			ClickConfirm(SHANDGST data, GSTFLAG flag);
		void			ResetCtn();
		void			GestureOut(sGesture &g);

		sGesture						m_gstout;
		bool							m_c_flag;//confirm��flag
		SHANDGST						m_sav;
		sGestureParamDClick				*m_param;

	};
}
#endif
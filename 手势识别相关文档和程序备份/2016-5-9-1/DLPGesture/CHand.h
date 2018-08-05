//单手执行类  调用后在一个完整手势前不能释放

#pragma once

#include "InnerStructure.h"

#ifndef CHAND_H
#define CHAND_H

namespace GestureEng
{
	class CHand
	{
	public:
		CHand();
		SHANDGST		GstTell(const sAction &r);
		void			ResetFlag_all();
		void			SetBasicParam(const sGestureParam *gc);

	private:
		void			DataPre(const sAction &r);
		void			SetFlag();
		void			Send(DATAUSF &data, const sAction &r);
		void			ResetFlag();

		SHANDGST		m_shandgst;
		HANDFLAG		m_hflag;
		DATAUSF			m_p1;
		DATAUSF			m_p2;
		DATAUSF			m_sav;
		sGestureParam	m_param;
	};
	typedef std::shared_ptr<CHand>	PHand;
}
#endif
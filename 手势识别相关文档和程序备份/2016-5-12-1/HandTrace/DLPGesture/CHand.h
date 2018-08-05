//����ִ����  ���ú���һ����������ǰ�����ͷ�

#pragma once

#include "InnerStructure.h"
#include <queue>

#ifndef CHAND_H
#define CHAND_H

namespace GestureEng
{
	class CHand
	{
	public:
		CHand();
		SHANDGST		GstTell(sAction r);
		void			ResetFlag_all();
		void			SetBasicParam(const sGestureParam *gc);

	private:
		void			DataPre(sAction r);
		void			SetFlag();
		void			Send(DATAUSF &data, sAction r);
		void			ResetFlag();

		SHANDGST		m_shandgst;
		HANDFLAG		m_hflag;
		DATAUSF			m_p1;
		DATAUSF			m_p2;
		DATAUSF			m_sav;
		sGestureParam	m_param;
	};
}
#endif
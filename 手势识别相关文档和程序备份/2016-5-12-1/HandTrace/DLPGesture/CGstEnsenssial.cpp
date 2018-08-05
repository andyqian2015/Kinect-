#include "stdafx.h"
#include "CGstEnsenssial.h"

#define HAND_1ST 0
#define HAND_2ND 1

namespace GestureEng
{
	CGstEnsenssial::CGstEnsenssial()
	{
		m_sg_1st.overflag = false;
		m_sg_1st.pos_ori.x = 0;
		m_sg_1st.pos_ori.y = 0;
		m_sg_2nd = m_sg_1st;
		m_sg_over.gtype = DualHand;
		m_sg_over.pos.x = 0;
		m_sg_over.pos.y = 0;
		m_sg_over.t = 0;
		m_sg_over.fScale = 1;
		m_sg_over.fScreenPivot.x = 0;
		m_sg_over.fScreenRotateAngle = 0;
		m_gstflag.dhand_flag = false;
		m_gstflag.click_flag = false;
		m_gstflag.move_flag = false;
		m_gstflag.push_flag = false;
		m_gstflag.over_flag = false;
		hand_1st = NULL;
		hand_2nd = NULL;
		m_pparam = new sGestureParam;
	}

	CGstEnsenssial::~CGstEnsenssial()
	{
		delete m_pparam;
		delete hand_1st;
		delete hand_2nd;
	}

	bool CGstEnsenssial::Equal(GSTFLAG a, GSTFLAG b)
	{
		return ((a.dhand_flag == b.dhand_flag)&(a.click_flag == b.click_flag)&(a.move_flag == b.move_flag)&(a.push_flag == b.push_flag)&(a.over_flag == b.over_flag));
	}

	bool CGstEnsenssial::OnAction(sAction r, sGesture &s)
	{
		GstTell(r, s);
		return true;
	}

	void CGstEnsenssial::SetGestureParam(const sGestureParam *gc)
	{
		m_pparam->time_max = gc->time_max;
		m_pparam->radius_move = gc->radius_move;
		m_pparam->time_longpush=gc->time_longpush;
	}

	void CGstEnsenssial::DataRecieve(sAction r, sGesture &g)
	{
		if (HAND_1ST == r.ID)
		{
			if (!hand_1st)
			{
				hand_1st = new CHand;
				if (hand_1st)
				{
					hand_1st->SetBasicParam(m_pparam);
				}
			}
			m_sg_1st = hand_1st->GstTell(r);
			//SetFlag();

			if (m_sg_1st.overflag)
			{
				if (hand_1st)
				    hand_1st->ResetFlag_all();
			}
		}
		else if (HAND_2ND == r.ID)
		{
			if (!m_gstflag.dhand_flag)
			{
				CleanQue(g);
				m_gstflag.dhand_flag = true;
			}
			if (!hand_2nd)
			{
				hand_2nd = new CHand;
				if (hand_2nd)
				{
					hand_2nd->SetBasicParam(m_pparam);
				}
			}
			m_sg_2nd = hand_2nd->GstTell(r);
			//SetFlag();

			if (m_sg_2nd.overflag)
			{
				if (hand_2nd)
				      hand_2nd->ResetFlag_all();
			}
		}
		SetFlag();
		// 	if (m_gstflag.over_flag)
		// 	{
		// 		ResetFlag();
		// 	}
	}

	sGestureParam* CGstEnsenssial::GetBasicParam()
	{
		return m_pparam;
	}

	void CGstEnsenssial::ResetParam()
	{
		m_pparam->radius_move = 10;
		m_pparam->time_longpush = 1000;
		m_pparam->time_max = 120000;
	}

	void CGstEnsenssial::SetFlag()
	{
		if (GestureEng::Click == m_sg_1st.st)
		{
			m_gstflag.click_flag = true;
		}
		if (GestureEng::Moveto == m_sg_1st.st)
		{
			m_gstflag.move_flag = true;
		}
		if (GestureEng::Lpush == m_sg_1st.st)
		{
			m_gstflag.push_flag = true;
		}
		if (m_sg_1st.overflag)
		{
			m_gstflag.over_flag = true;
		}

	}

	void CGstEnsenssial::ResetFlag()
	{
		m_sg_1st.overflag = false;
		m_sg_1st.pos_ori.x = 0;
		m_sg_1st.pos_ori.y = 0;
		m_sg_1st.t=0;
		m_sg_1st.st = Neither;
		m_sg_2nd = m_sg_1st;
		m_sg_over.gtype = DualHand;
		m_sg_over.pos.x = 0;
		m_sg_over.pos.y = 0;
		m_sg_over.t = 0;
		m_sg_over.fScale = 1;
		m_sg_over.fScreenPivot.x = 0;
		m_sg_over.fScreenRotateAngle = 0;
		m_gstflag.dhand_flag = false;
		m_gstflag.click_flag = false;
		m_gstflag.move_flag = false;
		m_gstflag.push_flag = false;
		m_gstflag.over_flag = false;
	}

	void CGstEnsenssial::CleanQue(sGesture &g)
	{
		g = m_sg_over;
	}
}

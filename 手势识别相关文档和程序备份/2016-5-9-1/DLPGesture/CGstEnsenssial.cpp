#include "stdafx.h"
#include "CGstEnsenssial.h"

#define HAND_1ST 0
#define HAND_2ND 1

namespace GestureEng
{
	CGstEnsenssial::CGstEnsenssial()
	{
// 		m_sg_1st.overflag = false;
// 		m_sg_1st.pos_ori.x = 0;
// 		m_sg_1st.pos_ori.y = 0;
// 		m_sg_2nd = m_sg_1st;
		m_sg_over.gtype = DualHand;
		m_sg_over.pos.x = 0;
		m_sg_over.pos.y = 0;
		m_sg_over.t = 0;
		m_sg_over.fScale = 1;
		m_sg_over.fScreenPivot.x = 0;
		m_sg_over.fScreenRotateAngle = 0;
// 		m_gstflag.dhand_flag = false;
// 		m_gstflag.click_flag = false;
// 		m_gstflag.move_flag = false;
// 		m_gstflag.push_flag = false;
// 		m_gstflag.over_flag = false;
		m_pparam = new sGestureParam;
	}

	CGstEnsenssial::~CGstEnsenssial()
	{
		delete m_pparam;
	}

	bool CGstEnsenssial::Equal(const GSTFLAG &a, const GSTFLAG &b)
	{
		return ((a.dhand_flag == b.dhand_flag)&&(a.click_flag == b.click_flag)&&(a.move_flag == b.move_flag)&&(a.push_flag == b.push_flag)&&(a.over_flag == b.over_flag));
	}

	bool CGstEnsenssial::OnAction(const std::vector<sAction> &r, sGesture &s)
	{
		GstTell(r, s);
		return true;
	}

	void CGstEnsenssial::SetGestureParam(const sGestureParam *gc)
	{
		m_pparam->time_max = gc->time_max;
		m_pparam->radius_move = gc->radius_move;
		m_pparam->time_longpush=gc->time_longpush;
		for (int i = 0; i != m_phands.size(); ++i)
		{
			m_phands[i]->SetBasicParam(m_pparam);
		}
	}

	void CGstEnsenssial::DataRecieve(const std::vector<sAction> &r, sGesture &g)
	{
		int count = 0;
		for (int i = 0; i != r.size(); ++i)
		{
			if (count<r[i].ID)count = r[i].ID;
		}
		++count;
		while (count > m_phands.size())
		{
			m_phands.push_back(PHand(new CHand));
			m_phands.back()->SetBasicParam(m_pparam);
			GestureEng::SHANDGST sg;
			m_sgs.push_back(sg);
			GestureEng::GSTFLAG gstflag;
			m_gstflags.push_back(gstflag);
		}
		if (r.size() > 1)m_gstflags[0].dhand_flag = true;
		else m_gstflags[0].dhand_flag = false;
		
		for (int i = 0; i != r.size(); ++i)
		{
			m_sgs[r[i].ID]=m_phands[r[i].ID]->GstTell(r[i]);
			if (m_sgs[r[i].ID].overflag)
				m_phands[r[i].ID]->ResetFlag_all();
			SetFlag(m_sgs[r[i].ID],m_gstflags[r[i].ID]);
		}
// 		if (HAND_1ST == r.ID)
// 		{
// 			if (phands.size()<HAND_1ST)
// 			{
// 				hand_1st = new CHand;
// 				if (hand_1st)
// 				{
// 					hand_1st->SetBasicParam(m_pparam);
// 				}
// 			}
// 			m_sg_1st = hand_1st->GstTell(r);
// 			//SetFlag();
// 
// 			if (m_sg_1st.overflag)
// 			{
// 				hand_1st->ResetFlag_all();
// 			}
// 			SetFlag(m_sg_1st);
// 		}
// 		else if (HAND_2ND == r.ID)
// 		{
// 			if (!m_gstflag.dhand_flag)
// 			{
// 				CleanQue(g);
// 				m_gstflag.dhand_flag = true;
// 			}
// 			if (!hand_2nd)
// 			{
// 				hand_2nd = new CHand;
// 				if (hand_2nd)
// 				{
// 					hand_2nd->SetBasicParam(m_pparam);
// 				}
// 			}
// 			m_sg_2nd = hand_2nd->GstTell(r);
// 			//SetFlag();
// 
// 			if (m_sg_2nd.overflag)
// 			{
// 				hand_2nd->ResetFlag_all();
// 			}
// 			SetFlag(m_sg_2nd);
// 		}
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

	void CGstEnsenssial::SetFlag(const GestureEng::SHANDGST &sg, GestureEng::GSTFLAG &flag)
	{
		if (GestureEng::Click == sg.st)
		{
			flag.click_flag = true;
		}
		if (GestureEng::Moveto == sg.st)
		{
			flag.move_flag = true;
		}
		if (GestureEng::Lpush == sg.st)
		{
			flag.push_flag = true;
		}
		if (sg.overflag)
		{
			flag.over_flag = true;
		}

	}

	void CGstEnsenssial::ResetFlag(SHANDGST &sg, GSTFLAG &flag)
	{
		sg.overflag = false;
		sg.pos_ori.x = 0;
		sg.pos_ori.y = 0;
		sg.t=0;
		sg.st = Neither;
//		m_sg_2nd = m_sg_1st;
		m_sg_over.gtype = DualHand;
		m_sg_over.pos.x = 0;
		m_sg_over.pos.y = 0;
		m_sg_over.t = 0;
		m_sg_over.fScale = 1;
		m_sg_over.fScreenPivot.x = 0;
		m_sg_over.fScreenRotateAngle = 0;
		flag.dhand_flag = false;
		flag.click_flag = false;
		flag.move_flag = false;
		flag.push_flag = false;
		flag.over_flag = false;
	}

	void CGstEnsenssial::CleanQue(sGesture &g)
	{
		g = m_sg_over;
	}
}

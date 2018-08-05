#include "stdafx.h"
#include "CHand.h"

namespace GestureEng
{

	CHand::CHand()
	{
		m_p1.FLAG = false;
		m_p1.usfdata.ID = 0;
		m_p1.usfdata.pos.x = 0;
		m_p1.usfdata.pos.y = 0;
		m_p1.usfdata.st = None;
		m_p1.usfdata.t = 0;
		m_p2 = m_p1;
		m_sav = m_p1;
		m_hflag.flag_down_trig = false;
		m_hflag.flag_move_trig = false;
		m_hflag.flag_up_trig = false;
		m_shandgst.overflag = false;
		m_shandgst.pos_ori.x = 0;
		m_shandgst.pos_ori.y = 0;
		m_shandgst.pos_fin = m_shandgst.pos_ori;
		m_shandgst.st = Neither;
		m_shandgst.t = 0;
		m_shandgst.t_ori = 0;
	}



	void CHand::SetFlag()
	{
		if (m_sav.FLAG)
		{
			if (Down == m_sav.usfdata.st)
			{
				m_hflag.flag_down_trig = true;
			}
			else if (Move == m_sav.usfdata.st)
			{
				m_hflag.flag_move_trig = true;
			}
		}
		if (m_p2.FLAG)
		{
			m_hflag.flag_up_trig = true;
		}

	}

	void CHand::SetBasicParam(const sGestureParam *gc)
	{
		m_param.time_max = gc->time_max;
		m_param.radius_move = gc->radius_move;
		m_param.time_longpush=gc->time_longpush;
	}

	void CHand::DataPre(const sAction &r)
	{
		if ((!m_p1.FLAG)&&(r.st == Down))
		{
			Send(m_p1, r);
		}
		else if ((m_p1.FLAG)&&(m_p1.usfdata.st == Down))
		{
			UINT64 timer= r.t - m_p1.usfdata.t;
			if (timer < m_param.time_max)
			{
				if ((Down == r.st))//若要实现移动中再长按的功能可修改此判断标准
				{
					timer = r.t - m_p1.usfdata.t;
					if (timer < m_param.time_longpush)
					{

					}
					else
					{
						if (r.t != m_p1.usfdata.t)
						{
							//Send(p2,r);
							Send(m_sav, r);
						}
					}

				}
				else if (Move == r.st)
				{
					//Send(p2,r);
					Send(m_sav, r);
				}
				else if (Up == r.st)
				{
					Send(m_p2, r);
				}
			}
			else
			{
				ResetFlag_all();
			}

		}
	}

	void CHand::ResetFlag()
	{
		m_p1.FLAG = false;
		m_sav.FLAG = false;
		m_p2.FLAG = false;
		//m_shandgst.st = inner::Neither;
	}

	void CHand::ResetFlag_all()
	{
		m_shandgst.st = Neither;
		m_shandgst.overflag = false;
		m_hflag.flag_down_trig = false;
		m_hflag.flag_move_trig = false;
		m_hflag.flag_up_trig = false;
		m_p1.FLAG = false;
		m_p2.FLAG = false;
		m_sav.FLAG = false;
	}


	SHANDGST CHand::GstTell(const sAction &r)
	{
		DataPre(r);
		SetFlag();
		if (m_p1.FLAG&&(m_sav.FLAG || m_p2.FLAG))
		{
			if (m_hflag.flag_move_trig&&(!m_hflag.flag_up_trig))
			{
				m_shandgst.st = Moveto;
				m_shandgst.t_ori = m_p1.usfdata.t;
				m_shandgst.t = m_sav.usfdata.t;
				m_shandgst.pos_ori = m_p1.usfdata.pos;
				m_shandgst.pos_fin = m_sav.usfdata.pos;
			}
			else if ((m_hflag.flag_up_trig) && ((!m_hflag.flag_move_trig)&&(!m_hflag.flag_down_trig)))
			{
				m_shandgst.st = Click;
				m_shandgst.t_ori = m_p1.usfdata.t;
				m_shandgst.t = m_p2.usfdata.t;
				m_shandgst.pos_ori = m_p1.usfdata.pos;
				m_shandgst.pos_fin = m_p2.usfdata.pos;
				ResetFlag();
				m_hflag.flag_up_trig = false;
			}
			else if (m_hflag.flag_down_trig&&(!m_hflag.flag_up_trig))
			{
				m_shandgst.st = Lpush;
				m_shandgst.t_ori = m_p1.usfdata.t;
				m_shandgst.t = m_sav.usfdata.t;
				m_shandgst.pos_ori = m_p1.usfdata.pos;
				m_shandgst.pos_fin = m_sav.usfdata.pos;
			}
			if ((m_hflag.flag_up_trig)&&(m_hflag.flag_move_trig || m_hflag.flag_down_trig))
			{
				m_shandgst.overflag = true;
				m_shandgst.st = Neither;
				m_shandgst.t_ori = m_p1.usfdata.t;
				m_shandgst.t = m_sav.usfdata.t;
				m_shandgst.pos_ori = m_p1.usfdata.pos;
				m_shandgst.pos_fin = m_p2.usfdata.pos;
				ResetFlag();
				m_hflag.flag_up_trig = false;
			}
			return m_shandgst;
 		}
		else//测试返回数据
		{
			m_shandgst.st = Neither;
			return m_shandgst;
		}

	}

	void CHand::Send(GestureEng::DATAUSF &data, const sAction &r)
	{
		data.FLAG = true;
		data.usfdata.ID = r.ID;
		data.usfdata.pos = r.pos;
		data.usfdata.st = r.st;
		data.usfdata.t = r.t;
	}

}

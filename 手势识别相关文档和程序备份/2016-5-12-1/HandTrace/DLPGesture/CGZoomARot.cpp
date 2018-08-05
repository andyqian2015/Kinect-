#include "stdafx.h"
#include "CGZoomARot.h"
#include "CHand.h"

CGZoomARot::CGZoomARot() :CGstEnsenssial()
{
	m_gstout.gtype = GNone;
	m_gstout.pos.x = 0.0;
	m_gstout.pos.y = 0.0;
	m_gstout.t = 0;
	m_gstout.fScale = 1;
	m_gstout.fScreenRotateAngle = 0;
	m_gstout.fScreenPivot.x = 0.0;
	m_gstout.fScreenPivot.y = 0.0;
	m_c_point_sav.x=0;
	m_c_point_sav.y = 0;
	m_c_point=m_c_point_sav;
	m_angle_sav=0;
	m_angle=0;
	m_prop_sav=0;
	m_prop=0;
}

// void CGZoomARot::DataRecieve(sAction r)
// {
// 	 CHand hand_1st;
// 	 if(1==r.ID)
// 	 {
// 	 	sg_1st = hand_1st.GstTell(r);
// 	 	SetFlag();
// 			
// 	 	if(sg_1st.overflag)
// 		{
// 			hand_1st.ResetFlag_all();
// 	 	}
//  	}
//  	else if (2==r.ID)
//  	{
//  		gstflag.dhand_flag=true;
// 		CleanQue(g);
//  		CHand hand_2nd;
//  		sg_2nd = hand_2nd.GstTell(r);
//  
//  		
//  		if(sg_2nd.overflag)
//  		{
//  			hand_2nd.ResetFlag_all();
//  		}
//  	}
// 	//这里调用各种单手手势engine 根据不同类型传入不同参数
// 	//这里调用双手手势engine
// 	 
//  	if (gstflag.over_flag)
//  	{
//  		ResetFlag();
//  	}
// }

void CGZoomARot::GstTell(sAction r, sGesture &g)
{
	DataRecieve(r, g);
	if (m_gstflag.dhand_flag)
	{
		if(m_gstflag.move_flag)
		{
			if ((m_sg_1st.st)&(m_sg_2nd.st))
			{
				ZoomOrRot(m_sg_1st,m_sg_2nd,g);
			}
		}
		m_c_point_sav=CentrePoint(m_sg_1st,m_sg_2nd);
		m_angle_sav=Angle(m_sg_1st,m_sg_2nd);
		m_dist_sav=Distance(m_sg_1st,m_sg_2nd);
	}
	if (m_gstflag.over_flag)
	{
		//置手势结束标志
		ResetCtn();
		ResetFlag();
	}
}

void CGZoomARot::SetFlag()
{
	if (inner::Click == m_sg_1st.st | inner::Click == m_sg_2nd.st)
	{
		m_gstflag.click_flag = true;
	}
	else if (inner::Moveto == m_sg_1st.st | inner::Moveto == m_sg_2nd.st)
	{
		m_gstflag.move_flag = true;
	}
	else if (inner::Lpush == m_sg_1st.st | inner::Lpush == m_sg_2nd.st)
	{
		m_gstflag.push_flag = true;
	}
	else if (m_sg_1st.overflag | m_sg_2nd.overflag)
	{
		m_gstflag.over_flag = true;
	}

}

void CGZoomARot::ZoomOrRot(inner::SHANDGST data1,inner::SHANDGST data2,sGesture &g)
{
	m_angle=Angle(data1,data2)-m_angle_sav;
	m_dist=Distance(data1,data2);
	m_prop=m_dist/m_dist_sav;
	if (m_prop!=1)
	{
		m_c_point=CentrePoint(data1,data2);
		m_gstout.gtype=GZoom;
		m_gstout.fScale=m_prop;
		m_gstout.pos=m_c_point;
		m_gstout.fScreenPivot=m_c_point_sav;
		if (data1.t<data2.t)
		{	
			m_gstout.t = data2.t;
		}
		else
		{
			m_gstout.t = data1.t;
		}
		GestureOut(g);
	}
	if (m_angle!=0)
	{
		m_c_point=CentrePoint(data1,data2);
		m_gstout.gtype=GRotate;
		m_gstout.fScreenRotateAngle=m_angle;
		m_gstout.pos=m_c_point;
		m_gstout.fScreenPivot = m_c_point_sav;
		if (data1.t < data2.t)
		{
			m_gstout.t = data2.t;
		}
		else
		{
			m_gstout.t = data1.t;
		}
		GestureOut(g);
	}

}

double CGZoomARot::Angle(inner::SHANDGST data1,inner::SHANDGST data2)
{
	double a;
	a=std::atan2((data2.pos_fin.y-data1.pos_fin.y),(data2.pos_fin.x-data1.pos_fin.x));
	return a;
}

long double CGZoomARot::Distance(inner::SHANDGST data1,inner::SHANDGST data2)
{
	long double a;
	a=std::sqrtl((data2.pos_fin.y-data1.pos_fin.y)*(data2.pos_fin.y-data1.pos_fin.y)+(data2.pos_fin.x-data1.pos_fin.x)*(data2.pos_fin.x-data1.pos_fin.x));
	return a;
}

Vec3<float> CGZoomARot::CentrePoint(inner::SHANDGST data1, inner::SHANDGST data2)
{
	Vec3<float> p;
	p.x=(data1.pos_fin.x+data2.pos_fin.x);
	p.y=(data1.pos_fin.y+data2.pos_fin.y);
	return p;
}

void CGZoomARot::ResetCtn()
{
	m_gstout.gtype = GNone;
	m_gstout.pos.x = 0.0;
	m_gstout.pos.y = 0.0;
	m_gstout.t = 0;
	m_gstout.fScale = 1;
	m_gstout.fScreenRotateAngle = 0;
	m_gstout.fScreenPivot.x = 0.0;
	m_gstout.fScreenPivot.y = 0.0;
	m_c_point.x=0;
	m_c_point.y = 0;
	m_angle=0;
	m_prop=0;
}

void CGZoomARot::GestureOut(sGesture &g)
{
	g = m_gstout;
}
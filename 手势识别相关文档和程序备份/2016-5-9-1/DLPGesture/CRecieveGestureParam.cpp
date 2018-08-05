#include "stdafx.h"
#include "CRecieveGestureParam.h"

// 
// typedef PIGesture(*IpGst)();

#pragma comment(lib,"DLPGesturex64d.lib")

extern GstRcnApi PIGesture CreateGesture();

GestureEng::PIGestureParam CreateParamPointer()
{
	GestureEng::PIGestureParam pp(new CRecieveGestureParam);
	return pp;
}

CRecieveGestureParam::CRecieveGestureParam()
{
// 	HINSTANCE hDll;
// 	IpGst p_gst;
// 	hDll = LoadLibrary(L"DLPGesturex64d.dll");
// 	
// 	if (hDll)	
// 	{
// 		p_gst =(IpGst) GetProcAddress(hDll, "CreateGesture");
// 		if (p_gst)
// 		{
// 			gesture = p_gst();
// 		}
// 	}
	m_basicparam = new sGestureParam;
	m_clickparam = new GestureEng::sGestureParamClick;
	m_dclickparam = new GestureEng::sGestureParamDClick;
	m_flipparam = new GestureEng::sGestureParamFlip;
	m_slidebackparam = new GestureEng::sGestureParamSlideBack;
	m_zarparam = new GestureEng::sGestureParamZoomARoatate;
	
		gesture = CreateGesture();
		if (!gesture)
		{
			//µ¯³ö´íÎóÐÅÏ¢
			exit(1);
		}
}

CRecieveGestureParam::~CRecieveGestureParam()
{
	delete m_basicparam;
	delete m_clickparam;
	delete m_dclickparam;
	delete m_flipparam;
	delete m_slidebackparam;
	delete m_zarparam;
}

bool CRecieveGestureParam::GetChecked(eGestureType type)
{
	return gesture->GetChecked(type);
}


void CRecieveGestureParam::Recognize(const sAction &r)
{
	gesture->OnAction(r, m_g);
}

std::vector<sGesture> CRecieveGestureParam::GetGesture()
{
	return m_g;
}

bool CRecieveGestureParam::SetGestureParam(UINT64 time, float radius,UINT64 time_l)
{
	m_basicparam->type = GMoveTo;
	m_clickparam->type = GClick;
	m_dclickparam->type = GDoubleClick;
	m_flipparam->type = GFlipDown;
	m_slidebackparam->type = GSwithHori;
	m_zarparam->type = GZoom;
	m_clickparam->time_longpush = m_dclickparam->time_longpush = m_flipparam->time_longpush = m_basicparam->time_longpush = m_slidebackparam->time_longpush = m_zarparam->time_longpush = time_l;
	m_clickparam->time_max = m_dclickparam->time_max = m_flipparam->time_max = m_basicparam->time_max = m_slidebackparam->time_max = m_zarparam->time_max = time;
	m_clickparam->radius_move = m_dclickparam->radius_move = m_flipparam->radius_move = m_basicparam->radius_move = m_slidebackparam->radius_move = m_zarparam->radius_move = radius;
	gesture->SetGestureMode(m_basicparam);
	gesture->SetGestureMode(m_clickparam);
	gesture->SetGestureMode(m_dclickparam);
	gesture->SetGestureMode(m_flipparam);
	gesture->SetGestureMode(m_slidebackparam);
	gesture->SetGestureMode(m_zarparam);
	return true;
}

void CRecieveGestureParam::ResetParam()
{
	gesture->Reset();
}

bool CRecieveGestureParam::SetGestureParamClick(float radius)
{
	m_clickparam->radius_click = radius;
	gesture->SetGestureMode(m_clickparam);
	return true;
}

bool CRecieveGestureParam::SetGestureParamDClick(UINT64 time, float radius)
{
	m_dclickparam->time_dclick = time;
	m_dclickparam->radius_dclick = radius;
	gesture->SetGestureMode(m_dclickparam);
	return true;
}

bool CRecieveGestureParam::SetGestureParamFlip(double speed, float distance)
{
	m_flipparam->speed_flip = speed;
	m_flipparam->distance_flip = distance;
	gesture->SetGestureMode(m_flipparam);
	return true;
}

bool CRecieveGestureParam::SetGestureParamSlideBack(double proportion, float distance, float distance_back)
{
	m_slidebackparam->proportion_back = proportion;
	m_slidebackparam->distance_back = distance_back;
	m_slidebackparam->distance_slide = distance;
	gesture->SetGestureMode(m_slidebackparam);
	return true;
}

bool CRecieveGestureParam::SetGestureParamZoomARoatate(double angle, double proportion)
{
	m_zarparam->angle_rotate = angle;
	m_zarparam->proportion_zoom = proportion;
	gesture->SetGestureMode(m_zarparam);
	return true;
}

bool CRecieveGestureParam::SetDisable(eGestureType type, bool Enable)
{
	
	gesture->EnableGesture(type, Enable);
	return true;
}

void CRecieveGestureParam::GetParamValue()
{
	if (gesture->GetParamValue(GMoveTo))
	{
		*m_basicparam = *(sGestureParam*)gesture->GetParamValue(GMoveTo);
	}
	if (gesture->GetParamValue(GClick))
	{
		*m_clickparam = *(GestureEng::sGestureParamClick*)gesture->GetParamValue(GClick);
	}
	if (gesture->GetParamValue(GDoubleClick))
	{
		*m_dclickparam = *(GestureEng::sGestureParamDClick*)gesture->GetParamValue(GDoubleClick);
	}
	if (gesture->GetParamValue(GFlipRight))		
	{
		*m_flipparam = *(GestureEng::sGestureParamFlip*)gesture->GetParamValue(GFlipRight);
	}
	if (gesture->GetParamValue(GSwithHori))
	{
		*m_slidebackparam = *(GestureEng::sGestureParamSlideBack*)gesture->GetParamValue(GSwithHori);
	}
	if (gesture->GetParamValue(GZoom))		
	{
		*m_zarparam = *(GestureEng::sGestureParamZoomARoatate*)gesture->GetParamValue(GZoom);
	}
	
}

sGestureParam*	CRecieveGestureParam::PassBasicParam()
{
	return m_basicparam;
}

GestureEng::sGestureParamClick*	CRecieveGestureParam::PassClickParam()
{
	return m_clickparam;
}

GestureEng::sGestureParamDClick*	CRecieveGestureParam::PassDClickParam()
{
	return m_dclickparam;
}

GestureEng::sGestureParamFlip*	CRecieveGestureParam::PassFlipParam()
{
	return m_flipparam;
}

GestureEng::sGestureParamSlideBack*	CRecieveGestureParam::PassBackParam()
{
	return m_slidebackparam;
}

GestureEng::sGestureParamZoomARoatate*	CRecieveGestureParam::PassZARParam()
{
	return m_zarparam;
}
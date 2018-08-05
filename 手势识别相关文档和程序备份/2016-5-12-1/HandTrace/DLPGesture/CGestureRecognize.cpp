#include "stdafx.h"
#include "CGestureRecognize.h"
#include "CHand.h"
//#include <boost/assert.hpp>

namespace GestureEng
{
	CGestureRecognize::CGestureRecognize()
	{
	
		//V2
//		m_PIKinectActionEvent = PIKinectActionEvent(new CKinectActionEvent(this));
		//V2
		 
		eGesture eg;

		eg.bEnable = false;
		eg.pEng = NULL;
		for (UINT i = 0; i < Gesture_COUNT; i++)
		{
			m_Gestures.push_back(eg);
		}
		m_Gestures[GNone].pEng = CreateCGNone();
// 		m_Gestures[0].bEnable = false;
// 		m_Gestures[0].pEng = CreateCGMoveTo();
// 		m_Gestures[1].bEnable = false;
// 		m_Gestures[1].pEng = CreateCGClick();
// 		m_Gestures[2].bEnable = false;
// 		m_Gestures[2].pEng = CreateCGLongPush();
// 		m_Gestures[5].bEnable = false;
// 		m_Gestures[5].pEng = CreateCGDClick();
// 		m_Gestures[6].bEnable = false;
// 		m_Gestures[6].pEng = CreateCGFlipRight();
// 		m_Gestures[7].bEnable = false;
// 		m_Gestures[7].pEng = CreateCGFlipLeft();
// 		m_Gestures[8].bEnable = false;
// 		m_Gestures[8].pEng = CreateCGSlideBackHori();
// 		m_Gestures[9].bEnable = false;
// 		m_Gestures[9].pEng = CreateCGFlipUp();
// 		m_Gestures[10].bEnable = false;
// 		m_Gestures[10].pEng = CreateCGFlipDown();
// 		m_Gestures[11].bEnable = false;
// 		m_Gestures[11].pEng = CreateCGSlideBackVert();
// 		m_Gestures[12].bEnable = false;
// 		m_Gestures[12].pEng = CreateCGDrag();
// 		m_Gestures[14].bEnable = false;
// 		m_Gestures[14].pEng = CreateCGZooming();
// 		m_Gestures[15].bEnable = false;
// 		m_Gestures[15].pEng = CreateCGRotate();

	}



	void CGestureRecognize::OnAction(const sAction &r, std::vector<sGesture> &g)
	{
		sGesture s;
		s.gtype = DualHand;
		for (UINT i = 0; i < m_Gestures.size(); ++i)
		{

			if (!m_Gestures[i].bEnable)
				continue;

			//BOOST_ASSERT(m_Gestures[i].pEng);
			if (m_Gestures[i].pEng && m_Gestures[i].pEng->OnAction(r, s))
			{
				if (s.gtype != DualHand)
				{
					g.push_back(s);
					s.gtype = DualHand;
				}
			}
		}

// 		if (g.size() > 10)
// 			AdjustHandGesture(g);
// 		if (!g.empty()) {
// 			m_vHandGesture = g;
// 			g.clear();
// 		}
	}

	//V2
// 	void CGestureRecognize::AdjustHandGesture(std::vector<sGesture> &handGesture)
// 	{
// // 		for (int i = 0; i < handGesture.size() - 1; i++)
// // 			handGesture[i] = handGesture[i + 1];
// // 		handGesture.pop_back();
// 		std::vector<sGesture> handGesture1(handGesture.size() - 1);
// 		for (int i = 0; i < handGesture1.size(); i++) {
// 			handGesture1[i] = handGesture[i + 1];
// 		}
// 		handGesture.clear();
// 		handGesture = handGesture1;
// 	}

	int CGestureRecognize::SetGestureMode(const sGestureParam *gc)
	{
		if (!gc)
		{
			return 0;
		}
		sGestureParamClick *gc_click=NULL;
		sGestureParamDClick *gc_dclick=NULL;
		sGestureParamFlip *gc_flip=NULL;
		sGestureParamSlideBack *gc_back=NULL;
		sGestureParamZoomARoatate *gc_zar=NULL;
		switch (gc->type)
		{
		case GMoveTo:
		case GLongPush:
		case GDrag:
			if (m_Gestures[GMoveTo].pEng)
				m_Gestures[GMoveTo].pEng->SetGestureParam(gc);
			if (m_Gestures[GLongPush].pEng)
				m_Gestures[GLongPush].pEng->SetGestureParam(gc);
			if (m_Gestures[GDrag].pEng)
				m_Gestures[GDrag].pEng->SetGestureParam(gc);
			break;
		case GClick:
			gc_click = (sGestureParamClick*)gc;
			if (m_Gestures[GClick].pEng)
				m_Gestures[GClick].pEng->SetClickParam(gc_click);
			break;
		case  GDoubleClick:
			gc_dclick = (sGestureParamDClick*)gc;
			if (m_Gestures[GDoubleClick].pEng)
				m_Gestures[GDoubleClick].pEng->SetDClickParam(gc_dclick);
			break;
		case GFlipRight:
		case GFlipDown:
		case GFlipUp:
		case GFlipLeft:
			gc_flip = (sGestureParamFlip*)gc;
			if (m_Gestures[GFlipRight].pEng)
				m_Gestures[GFlipRight].pEng->SetFlipParam(gc_flip);
			if (m_Gestures[GFlipLeft].pEng)
				m_Gestures[GFlipLeft].pEng->SetFlipParam(gc_flip);
			if (m_Gestures[GFlipDown].pEng)
				m_Gestures[GFlipDown].pEng->SetFlipParam(gc_flip);
			if (m_Gestures[GFlipUp].pEng)
				m_Gestures[GFlipUp].pEng->SetFlipParam(gc_flip);
			break;
		case GSwithHori:
		case GSwithVert:
			gc_back = (sGestureParamSlideBack*)gc;
			if (m_Gestures[GSwithHori].pEng)
				m_Gestures[GSwithHori].pEng->SetSlideBackParam(gc_back);
			if (m_Gestures[GSwithVert].pEng)
				m_Gestures[GSwithVert].pEng->SetSlideBackParam(gc_back);
			break;
		case GZoom:
		case GRotate:
			gc_zar = (sGestureParamZoomARoatate*)gc;
			if (m_Gestures[GZoom].pEng)
				m_Gestures[GZoom].pEng->SetZARParam(gc_zar);
			if (m_Gestures[GRotate].pEng)
				m_Gestures[GRotate].pEng->SetZARParam(gc_zar);
			break;
			
		default:
			break;
		}

		return 1;
	}

	void CGestureRecognize::EnableGesture(eGestureType gtype, bool bEnable)
	{
		switch (gtype)
		{
		case GMoveTo:
		{
						if (!m_Gestures[GMoveTo].pEng)
						{
							m_Gestures[GMoveTo].pEng = CreateCGMoveTo();
						}
						m_Gestures[GMoveTo].bEnable = bEnable;
						break;
		}
		case GClick:
		{
					   if (!m_Gestures[GClick].pEng)
					   {
						   m_Gestures[GClick].pEng = CreateCGClick();
					   }
						m_Gestures[GClick].bEnable = bEnable;
					   break;
		}
		case GLongPush:
		{
						  if (!m_Gestures[GLongPush].pEng)
						  {
							  m_Gestures[GLongPush].pEng = CreateCGLongPush();
						  }
							m_Gestures[GLongPush].bEnable = bEnable;
						  break;
		}
		case GDoubleClick:
		{
							 if (!m_Gestures[GDoubleClick].pEng)
							 {
								 m_Gestures[GDoubleClick].pEng = CreateCGDClick();
							 }
								 m_Gestures[GDoubleClick].bEnable = bEnable;
							 break;
		}
		case GFlipRight:
		{
						   if (!m_Gestures[GFlipRight].pEng)
						   {
							   m_Gestures[GFlipRight].pEng = CreateCGFlipRight();
						   }
							m_Gestures[GFlipRight].bEnable = bEnable;
						   break;
		}
		case GFlipLeft:
		{
						  if (!m_Gestures[GFlipLeft].pEng)
						  {
							  m_Gestures[GFlipLeft].pEng = CreateCGFlipLeft();
						  }
						  m_Gestures[GFlipLeft].bEnable = bEnable;
						  break;
		}
		case GSwithHori:
		{
						   if (!m_Gestures[GSwithHori].pEng)
						   {
							   m_Gestures[GSwithHori].pEng = CreateCGSlideBackHori();
						   }
						   m_Gestures[GSwithHori].bEnable = bEnable;
						   break;
		}
		case GFlipUp:
		{
						if (!m_Gestures[GFlipUp].pEng)
						{
							m_Gestures[GFlipUp].pEng = CreateCGFlipUp();
						}
						m_Gestures[GFlipUp].bEnable = bEnable;
						break;
		}
		case GFlipDown:
		{
						  if (!m_Gestures[GFlipDown].pEng)
						  {
							  m_Gestures[GFlipDown].pEng = CreateCGFlipDown();
						  }
						  m_Gestures[GFlipDown].bEnable = bEnable;
						  break;
		}
		case GSwithVert:
		{
						   if (!m_Gestures[GSwithVert].pEng)
						   {
							   m_Gestures[GSwithVert].pEng = CreateCGSlideBackVert();
						   }
						   m_Gestures[GSwithVert].bEnable = bEnable;
						   break;
		}
		case GDrag:
		{
					  if (!m_Gestures[GDrag].pEng)
					  {
						  m_Gestures[GDrag].pEng = CreateCGDrag();
					  }
					  m_Gestures[GDrag].bEnable = bEnable;
					  break;
		}
		case GZoom:
		{
					  if (!m_Gestures[GZoom].pEng)
					  {
						  m_Gestures[GZoom].pEng = CreateCGZooming();
					  }
					  m_Gestures[GZoom].bEnable = bEnable;
					  break;
		}
		case GRotate:
		{
						if (!m_Gestures[GRotate].pEng)
						{
							m_Gestures[GRotate].pEng = CreateCGRotate();
						}
						m_Gestures[GRotate].bEnable = bEnable;
						break;
		}

		default:
			break;
		}
		bool have;
		have = (m_Gestures[GMoveTo].bEnable | m_Gestures[GLongPush].bEnable | m_Gestures[GClick].bEnable | m_Gestures[GDoubleClick].bEnable | m_Gestures[GFlipDown].bEnable | m_Gestures[GFlipLeft].bEnable | m_Gestures[GFlipRight].bEnable | m_Gestures[GFlipUp].bEnable | m_Gestures[GSwithVert].bEnable | m_Gestures[GSwithHori].bEnable | m_Gestures[GDrag].bEnable | m_Gestures[GZoom].bEnable | m_Gestures[GRotate].bEnable);
		if (have)
		{
			m_Gestures[GNone].bEnable = true;
		}
			
	}

	void CGestureRecognize::Reset()
	{
		for (UINT i = 0; i < m_Gestures.size(); ++i)
		{
 			if (m_Gestures[i].pEng)
 			{
				m_Gestures[i].bEnable = false;
				m_Gestures[i].pEng->ResetParam();
 			}
		}
	}

	const sGestureParam* CGestureRecognize::GetParamValue(eGestureType type)
	{
		switch (type)
		{
		case GMoveTo:
			return GetBasicParam();
			break;
		case GClick:
			return GetClickParam();
			break;
		case GDoubleClick:
			return GetDClickParam();
			break;
		case GFlipRight:
			return GetFlipParam();
			break;
		case GSwithHori:
			return GetBackParam();
			break;
		case GZoom:
			return GetZARParam();
			break;
		default:
			return NULL;
			break;
		}
	}

	bool CGestureRecognize::GetChecked(eGestureType type)
	{
		switch (type)
		{
		case GMoveTo:
			return m_Gestures[GMoveTo].bEnable;
			break;
		case GClick:
			return m_Gestures[GClick].bEnable;
			break;
		case GLongPush:
			return m_Gestures[GLongPush].bEnable;
			break;
		case GNone:
			return m_Gestures[GNone].bEnable;
			break;
		case GSingleClick:
			return m_Gestures[GSingleClick].bEnable;
			break;
		case GDoubleClick:
			return m_Gestures[GDoubleClick].bEnable;
			break;
		case GFlipRight:
			return m_Gestures[GFlipRight].bEnable;
			break;
		case GFlipLeft:
			return m_Gestures[GFlipLeft].bEnable;
			break;
		case GSwithHori:
			return m_Gestures[GSwithHori].bEnable;
			break;
		case GFlipUp:
			return m_Gestures[GFlipUp].bEnable;
			break;
		case GFlipDown:
			return m_Gestures[GFlipDown].bEnable;
			break;
		case GSwithVert:
			return m_Gestures[GSwithVert].bEnable;
			break;
		case GDrag:
			return m_Gestures[GDrag].bEnable;
			break;
		case DualHand:
			return m_Gestures[DualHand].bEnable;
			break;
		case GZoom:
			return m_Gestures[GZoom].bEnable;
			break;
		case GRotate:
			return m_Gestures[GRotate].bEnable;
			break;

		default:
			return false;
			break;
		}
	}
	
	const sGestureParam*	CGestureRecognize::GetBasicParam()
	{
		for (UINT i = 0; i != m_Gestures.size();i++)
		{
			if (m_Gestures[i].pEng)
			{
				return m_Gestures[i].pEng->GetBasicParam();

			}
		}
		return NULL;
	}

	const sGestureParamClick*	CGestureRecognize::GetClickParam()
	{
		if (m_Gestures[GClick].pEng)
		{
			return m_Gestures[GClick].pEng->GetClickParam();

		}
		else
		{
			return NULL;
		}
	}

	const sGestureParamDClick* CGestureRecognize::GetDClickParam()
	{
		if (m_Gestures[GDoubleClick].pEng)
		{
			return m_Gestures[GDoubleClick].pEng->GetDclickParam();

		}
		else
		{
			return NULL;
		}
	}

	const sGestureParamFlip* CGestureRecognize::GetFlipParam()
	{
		if (m_Gestures[GFlipRight].pEng)
		{
			return	m_Gestures[GFlipRight].pEng->GetFlipParam();
		}
		else if (m_Gestures[GFlipLeft].pEng)
		{
			return m_Gestures[GFlipLeft].pEng->GetFlipParam();
		}
		else if (m_Gestures[GFlipUp].pEng)
		{
			return m_Gestures[GFlipUp].pEng->GetFlipParam();
		}
		else if (m_Gestures[GFlipDown].pEng)
		{
			return m_Gestures[GFlipDown].pEng->GetFlipParam();
		}
		else
		{
			return NULL;
		}
	}

	const sGestureParamSlideBack*	CGestureRecognize::GetBackParam()
	{
		if (m_Gestures[GSwithHori].pEng)
		{
			return m_Gestures[GSwithHori].pEng->GetSlideBackParam();

		}
		else if (m_Gestures[GSwithVert].pEng)
		{
			return m_Gestures[GSwithVert].pEng->GetSlideBackParam();
		}
		else
		{
			return NULL;
		}
	}

	const sGestureParamZoomARoatate*	CGestureRecognize::GetZARParam()
	{
		if (m_Gestures[GZoom].pEng)			
		{

			return m_Gestures[GZoom].pEng->GetZARParam();
		}
		else if (m_Gestures[GRotate].pEng)
		{
			return m_Gestures[GRotate].pEng->GetZARParam();
		}
		else
		{
			return NULL;
		}
	}

	//V2
// 	void CGestureRecognize::init(PIDLPUnknown pKinectActon)
// 	{
// 		m_PIKinectAction = GetDLPUnknown<IKinectAction>(pKinectActon);
// 		SetEventConnect();
// 	}

// 	BOOL CGestureRecognize::SetEventConnect()
// 	{
// 		if (!m_PIKinectAction)
// 			return  false;
// 
// 		BOOL bConnect = m_PIKinectAction->Connect(m_PIKinectActionEvent, true);
// 		if (bConnect)
// 		    m_ActionEventCount++;
// 		return bConnect;
// 	}

	CGestureRecognize::~CGestureRecognize()
	{
	//	DisEventConnect();
	}
	//V2


// 	void CGestureRecognize::DisEventConnect()
// 	{
// 		if (m_PIKinectAction)
// 		{
// 			while (m_ActionEventCount) {
// 				BOOL bflag = m_PIKinectAction->Connect(m_PIKinectActionEvent, false);
// 				if (bflag)
// 				    m_ActionEventCount--;
// 			}
// 				
// 
// 		}
// 	}


	//V2 Test
// 	void CGestureRecognize::Test(sGesture & s)
// 	{
// 
// 
// 		std::fstream outfile("D:\\handGesture.txt", std::ios::app);
// 		if (!m_vHandGesture.empty()) {
// 
// 			s = m_vHandGesture[m_vHandGesture.size()-1];
// 			outfile << m_vHandGesture[m_vHandGesture.size() - 1].gtype << " " << m_vHandGesture[m_vHandGesture.size() - 1].pos.x << " " << m_vHandGesture[m_vHandGesture.size() - 1].pos.y << " " << m_vHandGesture[m_vHandGesture.size() - 1].t << std::endl;
// 		}
// 
// 		else
// 			s = sGesture();
// 	}

	//V2

	// 设置手势属性，可以设置多个需要识别的手势，跟踪数据自动清零
	// int  CGstRcn::SetGestureMode(const GESTURE_CONFIG& gb)
	// {
	// 	switch (gb.gesture)
	// 	{
	// 	case eG_Click:
	// 		m_cfgClick.SetConfig(gb);
	// 		break;
	// 	case eG_Slide:
	// 		m_cfgSlide.SetConfig(gb);
	// 		break;
	// 	case eG_Rotate:
	// 		m_cfgRotate.SetConfig(gb);
	// 		break;
	// 	case eG_Scale:
	// 		m_cfgScale.SetConfig(gb);
	// 		break;
	// 	}
	// 	return true;
	// }
	// 
	// 
	// 
	// #define PI 3.14159265359f
	// 
	// void     CGstRcn::EnableGesture(eGesture gtype, bool bEnable)
	// {
	// 	switch (gtype)
	// 	{
	// 	case eG_Click:
	// 		m_cfgClick.m_bEnable = bEnable;
	// 		break;
	// 	case eG_Slide:
	// 		m_cfgSlide.m_bEnable = bEnable;
	// 		break;
	// 	case eG_Rotate:
	// 		m_cfgRotate.m_bEnable = bEnable;
	// 		break;
	// 	case eG_Scale:
	// 		m_cfgScale.m_bEnable = bEnable;
	// 		m_hgQuene[0].GetLastPos(m_ptScale[0]);
	// 		m_hgQuene[1].GetLastPos(m_ptScale[1]);
	// 		m_distScale = m_ptScale[0].Distance(m_ptScale[1]);
	// 		if (m_distScale == 0)
	// 		{
	// 			AtlTrace(_T("%g"));
	// 		}
	// 		m_rotAngle = 180 / PI*atan((m_ptScale[0].y - m_ptScale[1].y) / (m_ptScale[0].x - m_ptScale[1].x));
	// 		break;
	// 	}
	// }
	// 
	// 
	// 某个人的人手数据传入，得到特定手势状态，通常起始阶段（手势min_time时间内）不会产生手势，之后可能连续生成手势状态
	// int CGstRcn::OnAction(const sAction& ps)//物体接收到手动作后调用此接口函数
	// {
	// 	CAction person(ps);
	// 	m_hg.gtype = eG_unKnown;
	// 	if (!person.HasHands())
	// 		return -1;
	// 	//计算速度和加速度
	// 	if (ps.nID >= 0 || ps.nID < 2)
	// 		return GetQuene(ps.nID).Push(person);
	// 	return -1;
	// }
	// 
	// 
	// 手势识别终结销毁
	// void	CGstRcn::Destroy()
	// {
	// 	delete  this;
	// }
	// 
	// 检测滑动手势       //检测手势单击？//
	// bool CGstRcn::CheckClick()
	// {
	// 	if (m_cfgClick.m_bEnable == false)
	// 		return false;
	// 	for (int i = 0; i < 2; i++)
	// 	{
	// 		if (m_hgQuene[i].IsClicked(m_hg.pos))
	// 		{
	// 			m_hg.gtype = eG_Click;
	// 			m_hgQuene[i].Reset();
	// 			return true;
	// 		}
	// 	}
	// 
	// 
	// 	return false;
	// 
	// }
	// 
	// 检测滑动手势
	// bool CGstRcn::CheckSlide()
	// {
	// 	if (m_cfgSlide.m_bEnable == false)
	// 		return false;
	// 	for (int i = 0; i < 2; i++)
	// 	{
	// 		if (m_hgQuene[i].IsMoving(m_hg.pos))
	// 		{
	// 			m_hg.gtype = eG_Slide;
	// 			return true;
	// 		}
	// 	}
	// 	return 0;
	// }
	// 
	// 检测滑动手势          //检测手的旋转吧？//
	// bool CGstRcn::CheckRotate()
	// {
	// 	if (m_cfgRotate.m_bEnable == false)
	// 		return false;
	// 	return 0;
	// }
	// 
	// 检测缩放手势
	// bool CGstRcn::CheckScale()
	// {
	// 	if (m_cfgScale.m_bEnable == false)
	// 		return false;
	// 	CAction act[2];
	// 
	// 
	// 	if (m_hgQuene[0].IsMoving(act[0].vScreenPos))
	// 	{
	// 		if (m_hgQuene[1].IsMoving(act[1].vScreenPos))
	// 		{
	// 			m_hg.gtype = eG_Scale;
	// 			float dist = act[0].vScreenPos.Distance(act[1].vScreenPos);
	// 
	// 			m_hg.ScreenScaleDir = DLPHand::Point3f(act[0].vScreenPos.x - act[1].vScreenPos.x, act[0].vScreenPos.y - act[1].vScreenPos.y, 0);
	// 			float ref_dist = m_distScale + 200;
	// 			m_hg.fScale = 1.0f + (dist - m_distScale) / ref_dist;                                                   //获取图像缩放比例//
	// 			m_hg.fScreenRotateAngle = 180 / PI*atan(m_hg.ScreenScaleDir.y / m_hg.ScreenScaleDir.x) - m_rotAngle;  //获取到识别区域的旋转角//
	// 			if (m_hg.ScreenScaleDir.x < 0)
	// 				m_hg.fScreenRotateAngle += 180.0f;
	// 
	// 			TCHAR sinfo[255];
	// 			_stprintf_s(sinfo, 255, _T("m_distScale = %.2f,fScale = %.2f,(%.2f,%.2f),(%.2f,%.2f)\n"), m_distScale, m_hg.fScale,
	// 				act[0].vScreenPos.x, act[0].vScreenPos.y, act[1].vScreenPos.x, act[1].vScreenPos.y);
	// 			OutputDebugString(sinfo);
	// 			//AtlTrace("d=%.2f,s=%.2f,r=%.2f\n",dist,m_hg.fScale,m_hg.fScreenRotateAngle);
	// 			return true;
	// 		}
	// 
	// 	}
	// 
	// 
	// 	return 0;
	// }
	// 
	// 
	// const HandGesture& CGstRcn::Process()
	// {
	// 	m_hg.gtype = eG_unKnown;
	// 	if (CheckClick())
	// 		return m_hg;
	// 	if (CheckSlide())
	// 		return m_hg;
	// 	if (CheckScale())
	// 		return m_hg;
	// 	if (CheckRotate())
	// 		return m_hg;
	// 	return m_hg;
	// }
	// 
	// void	CGstRcn::Reset()
	// {
	// 	for (int i = 0; i < 2; i++)
	// 	{
	// 		m_hgQuene[i].Reset();
	// 	}
	// 
	// 	m_cfgClick.SetDefault(eG_Click);
	// 	m_cfgSlide.SetDefault(eG_Slide);
	// 	m_cfgScale.SetDefault(eG_Scale);
	// 	m_cfgRotate.SetDefault(eG_Rotate);
	// }
	// 
	// CHGQueue& CGstRcn::GetQuene(int id)
	// {
	// 	for (int i = 0; i < 2; i++)
	// 	{
	// 		if (m_hgQuene[i].GetID() == id)
	// 			return m_hgQuene[i];
	// 	}
	// 
	// 	return m_hgQuene[0].IsActive() ? m_hgQuene[1] : m_hgQuene[0];
	// }
}
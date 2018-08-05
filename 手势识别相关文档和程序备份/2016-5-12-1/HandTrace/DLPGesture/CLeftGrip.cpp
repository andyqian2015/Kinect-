#include "stdafx.h"
#include "CLeftGrip.h"

namespace SpatialGestureEng
{
	CLeftGrip::CLeftGrip()
	{
		m_hand_save = m_overflag = HandState_Unknown;
		m_time = 0;
	}

	void CLeftGrip::OnAction(IBody **pbody, SpatialGesture &sg)
	{
		for (UINT i = 0; i < BODY_COUNT; i++)
		{
			IBody *ppbody = pbody[i];
			BOOLEAN bTracked = false;
			HRESULT hResult = ppbody->get_IsTracked(&bTracked);
			if (SUCCEEDED(hResult) && bTracked)
			{
				HandState hand = HandState_Unknown;
				hResult = ppbody->get_HandLeftState(&hand);
				Joint joint[JointType::JointType_Count];
				hResult = ppbody->GetJoints(JointType::JointType_Count, joint);
				if (SUCCEEDED(hResult))
				{
					CameraSpacePoint point;
					point.X = (joint[JointType::JointType_HandLeft].Position.X + joint[JointType::JointType_HandTipLeft].Position.X) / 2;
					point.Y = (joint[JointType::JointType_HandLeft].Position.Y + joint[JointType::JointType_HandTipLeft].Position.Y) / 2;
					point.Z = (joint[JointType::JointType_HandLeft].Position.Z + joint[JointType::JointType_HandTipLeft].Position.Z) / 2;

					CameraSpacePoint hand_direction;
					hand_direction.X = 100.0*(joint[JointType::JointType_HandTipLeft].Position.X - joint[JointType::JointType_HandLeft].Position.X);
					hand_direction.Y = 100.0*(joint[JointType::JointType_HandTipLeft].Position.Y - joint[JointType::JointType_HandLeft].Position.Y);
					hand_direction.Z = 100.0*(joint[JointType::JointType_HandTipLeft].Position.Z - joint[JointType::JointType_HandLeft].Position.Z);

					if (hand != HandState_Closed & hand != HandState_NotTracked&hand != HandState_Unknown)
					{
						m_hand_save = hand;
						m_time = sg.time;

						if (m_overflag == HandState_Closed)
						{
							sg.type = Grip_LRelease;
							sg.position = point;
							sg.handDirection = hand_direction;
							m_overflag = HandState_Unknown;
						}
					}
					else if (m_hand_save == HandState_Open)
					{

						if (HandState_Closed == hand)
						{
							//传出抓取手势数据
							sg.type = L_GripTo;
							sg.position = point;
							sg.handDirection = hand_direction;
							m_overflag = HandState_Closed;
							m_time = sg.time;
						}
						else
						{
							if ((HandState_Closed == m_overflag))
							{
								if ((sg.time - m_time) < 10000000)
								{
									sg.type = L_GripTo;
									sg.position = point;
								}
								else
								{
									sg.type = Grip_LRelease;
									sg.position = point;
									sg.handDirection = hand_direction;
									m_overflag = HandState_Unknown;
									m_hand_save = HandState_Unknown;
								}

							}
						}
					}
				}
			}
		}
	}
}
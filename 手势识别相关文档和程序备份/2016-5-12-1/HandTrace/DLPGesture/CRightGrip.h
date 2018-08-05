#ifndef CRIGHTGRIP_H
#define CRIGHTGRIP_H

#include "PrivateStructure.h"

namespace SpatialGestureEng
{
	class CRightGrip :public ISpatialGesture
	{
	public:
		CRightGrip();
		void OnAction(IBody **ppbody, SpatialGesture &sg);
	private:
		HandState m_hand_save;
		HandState m_overflag;
		TIMESPAN  m_time;
	};
}

#endif
#ifndef CLEFTGRIP_H
#define CLEFTGRIP_H

#include "PrivateStructure.h"

namespace SpatialGestureEng
{
	class CLeftGrip :public ISpatialGesture
	{
	public:
		CLeftGrip();
		void OnAction(IBody **ppbody, SpatialGesture &sg);
	private:
		HandState m_hand_save;
		HandState m_overflag;
		TIMESPAN  m_time;
	};
}
#endif
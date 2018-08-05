#ifndef PRIVATESTRUCTURE_H
#define PRIVATESTRUCTURE_H

#include <DLPGesture/IDLPSpatialGesture.h>
#include <memory>

namespace SpatialGestureEng
{
	__interface ISpatialGesture
	{
		void OnAction(IBody **ppbody, SpatialGesture &sg);
	};
	typedef std::shared_ptr<ISpatialGesture> PISpatialGesture;
}

#endif
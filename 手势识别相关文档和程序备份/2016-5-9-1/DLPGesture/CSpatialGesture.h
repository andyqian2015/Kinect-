#ifndef CSPATIALGESTURE_H
#define CSPATIALGESTURE_H

#include "PrivateStructure.h"
#include <DLPGesture/IDLPSpatialGesture.h>
#include <vector>

namespace SpatialGestureEng
{
	class CAdvancedGesture :public IStartSpatialGesture
	{
	public:
		CAdvancedGesture();
		void OnAction(std::vector<SpatialGesture> &sg);
		void EnableGesture(GestureType type, bool bEnable);
	private:
		IKinectSensor		*m_pKinectSensor;
		ICoordinateMapper	*m_pCoordinateMapper;
		IBodyFrameReader	*m_pBodyFrameReader;

		struct sGesture
		{
			PISpatialGesture pEng;
			bool			 bEnable;
		};
		std::vector<sGesture> m_Gestures;

	};
}

#endif
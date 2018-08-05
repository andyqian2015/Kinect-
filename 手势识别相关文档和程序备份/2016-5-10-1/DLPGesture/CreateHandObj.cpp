
#include "stdafx.h"
#include "dlpgesture\IDLPGesture.h"
#include "CKinectAction.h"
#include "CKinectActionEvent.h"
#include "CKinectSensor.h"
#include "CHandTrace.h"
#include "CKinectSensor.h"
#include "CCalib.h"
#include "CEvent.h"
#include "CSynGesture.h"

using namespace GestureEng;


PIDLPUnknown GestureEng::CreateHandObj(REFIID objClassID)
{
	if (objClassID == __uuidof(GestureEng::IMyKinectSensor)) {
		PIDLPUnknown p = GestureEng::PIMyKinectSensor(new CKinectSensor);
			return p;
	}


	if (objClassID == __uuidof(GestureEng::IImgTransform)) {
		PIDLPUnknown p = GestureEng::PIImgTransform(new CKinectSensor);
		return p;
	}


	if (objClassID == __uuidof(GestureEng::IKinectSkeleton)) {
		PIDLPUnknown p = GestureEng::PIKinectSkeleton(new CKinectAction);
		return p;
	}

	if (objClassID == __uuidof(GestureEng::IKinectAction)) {
		PIDLPUnknown p = GestureEng::PIKinectAction(new CKinectAction);
		return p;
	}

	if (objClassID == __uuidof(GestureEng::IKinectActionEvent)) {
		PIDLPUnknown p = GestureEng::PIKinectActionEvent(new CKinectActionEvent);
		return p;
	}

	if (objClassID == __uuidof(GestureEng::IHandTrace)) {
		PIDLPUnknown p = GestureEng::PIHandTrace(new CHandTrace);
		return p;
	}

	if (objClassID == __uuidof(GestureEng::ICalib)) {
		PIDLPUnknown p = GestureEng::PICalib(new CCalib);
		return p;
	}

	if (objClassID == __uuidof(GestureEng::IEvent))  {
		PIDLPUnknown p = GestureEng::PIEvent(new CEvent);
		return p;
	}

	if (objClassID == __uuidof(GestureEng::ISynGesture))
	{
		PIDLPUnknown p = GestureEng::PISynGesture(new CSynGesture);
		return p;
	}

	return NULL;
}



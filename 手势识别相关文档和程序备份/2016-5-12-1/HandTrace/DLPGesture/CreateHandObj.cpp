
#include "stdafx.h"
#include "dlpgesture\IDLPGesture.h"
#include "CKinectAction.h"
#include "CKinectActionEvent.h"
#include "CKinectSensor.h"
#include "CHandTrace.h"
#include "CKinectSensor.h"
#include "CCalib.h"
#include "CEvent.h"


PIDLPUnknown CreateHandObj(REFIID objClassID)
{
	if (objClassID == __uuidof(IMyKinectSensor)) {
		PIDLPUnknown p = PIMyKinectSensor(new CKinectSensor);
			return p;
	}


	if (objClassID == __uuidof(IImgTransform)) {
		PIDLPUnknown p = PIImgTransform(new CKinectSensor); 
		return p;
	}


	if (objClassID == __uuidof(IKinectSkeleton)) {
		PIDLPUnknown p = PIKinectSkeleton(new CKinectAction);
		return p;
	}

	if (objClassID == __uuidof(IKinectAction)) {
		PIDLPUnknown p = PIKinectAction(new CKinectAction);
		return p;
	}

	if (objClassID == __uuidof(IKinectActionEvent)) {
		PIDLPUnknown p = PIKinectActionEvent(new CKinectActionEvent);
		return p;
	}

	if (objClassID == __uuidof(IHandTrace)) {
		PIDLPUnknown p = PIHandTrace(new CHandTrace);
		return p;
	}

	if (objClassID == __uuidof(ICalib)) {
		PIDLPUnknown p = PICalib(new CCalib);
		return p;
	}

	if (objClassID == __uuidof(IEvent))  {
		PIDLPUnknown p = PIEvent(new CEvent);
		return p;
	}

	return NULL;
}



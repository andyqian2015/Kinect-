#define GSTRCN_DLL

#include "stdafx.h"
#include "dlpgesture\IDLPGesture.h"
#include "CGestureRecognize.h"
#include <windows.h>
#include <memory>
#include "CGstEnsenssial.h"
#include "CGClick.h"
#include "CGDoubleClick.h"
#include "CGDrag.h"
#include "CGFlipUp.h"
#include "CGFlipDown.h"
#include "CGFlipLeft.h"
#include "CGFlipRight.h"
#include "CGLongPush.h"
#include "CGMoveTo.h"
#include "CGSlideBackHori.h"
#include "CGSlideBackVert.h"
#include "CGZooming.h"
#include "CGRotate.h"
// #include "DLPGesture/GestureParam.h"
// #include "CRecieveGestureParam.h"


namespace GestureEng
{
	PIGesture	CreateGesture()
	{
		PIGesture pc(new GestureEng::CGestureRecognize);
		return pc;
	}


	PCGstEnsenssial CreateCGClick()
	{
		PCGstEnsenssial pc(new CGClick);
		return pc;
	}

	PCGstEnsenssial CreateCGDClick()
	{
		PCGstEnsenssial pc(new CGDoubleClick);
		return pc;
	}

	PCGstEnsenssial CreateCGDrag()
	{
		PCGstEnsenssial pc(new CGDrag);
		return pc;
	}

	PCGstEnsenssial CreateCGFlipUp()
	{
		PCGstEnsenssial pc(new CGFlipUp);
		return pc;
	}

	PCGstEnsenssial CreateCGFlipDown()
	{
		PCGstEnsenssial pc(new CGFlipDown);
		return pc;
	}

	PCGstEnsenssial CreateCGFlipLeft()
	{
		PCGstEnsenssial pc(new CGFlipLeft);
		return pc;
	}

	PCGstEnsenssial CreateCGFlipRight()
	{
		PCGstEnsenssial pc(new CGFlipRight);
		return pc;
	}

	PCGstEnsenssial CreateCGLongPush()
	{
		PCGstEnsenssial pc(new CGLongPush);
		return pc;
	}

	PCGstEnsenssial CreateCGMoveTo()
	{
		PCGstEnsenssial pc(new CGMoveTo);
		return pc;
	}

	PCGstEnsenssial CreateCGSlideBackHori()
	{
		PCGstEnsenssial pc(new CGSlideBackHori);
		return pc;
	}

	PCGstEnsenssial CreateCGSlideBackVert()
	{
		PCGstEnsenssial pc(new CGSlideBackVert);
		return pc;
	}

	PCGstEnsenssial CreateCGZooming()
	{
		PCGstEnsenssial pc(new CGZooming);
		return pc;
	}

	PCGstEnsenssial CreateCGRotate()
	{
		PCGstEnsenssial pc(new CGRotate);
		return pc;
	}

	PCGstEnsenssial CreateCGNone()
	{
		PCGstEnsenssial pc(new CGNone);
		return pc;
	}
}
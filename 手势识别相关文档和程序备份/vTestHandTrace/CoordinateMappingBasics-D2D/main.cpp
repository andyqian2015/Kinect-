


#include <atlbase.h>

#include "stdafx.h"
#include <strsafe.h>
#include <math.h>
#include <limits>
#include <Wincodec.h>
#include "resource.h"
#include "CoordinateMappingBasics.h"
#include <fstream>
#include "Math.h"
#include "OpenExr/include/ImathVec.h"
#include "DepthBasics.h"
#include "ICalib.h"
#include "boost\smart_ptr.hpp"
using namespace IMATH_NAMESPACE;


#ifndef HINST_THISCOMPONENT
EXTERN_C IMAGE_DOS_HEADER __ImageBase;
#define HINST_THISCOMPONENT ((HINSTANCE)&__ImageBase)
#endif



int APIENTRY wWinMain(
	_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR lpCmdLine,
	_In_ int nShowCmd
	)
{
 	UNREFERENCED_PARAMETER(hPrevInstance);
 	UNREFERENCED_PARAMETER(lpCmdLine);

	
	V3f p1(426, 355, 1041), p2(361, 300, 1689), p3(161, 298, 1711), p4(100, 354, 1049);

	std::vector<V3f> vMarkKinectPos;
	vMarkKinectPos.push_back(p1);
	vMarkKinectPos.push_back(p2);
	vMarkKinectPos.push_back(p3);
	vMarkKinectPos.push_back(p4); 

	
	//按像素确定识别区域，离屏越近，可点击的点范围越大，离屏越远，可点击的范围越小//

	V3f vscreenpos1(0, 0, 0);
	V3f vscreenpos2(0, 650, 0);
	V3f vscreenpos3(920, 650, 0);
	V3f vscreenpos4(920, 0, 0);
	std::vector<V3f> vMarkScreenPos;
	vMarkScreenPos.push_back(vscreenpos1);
	vMarkScreenPos.push_back(vscreenpos2);
	vMarkScreenPos.push_back(vscreenpos3);
	vMarkScreenPos.push_back(vscreenpos4);
	float h = 500, h1 = 100, Radius =25;
	int KinectInstallPattern = 0;

	int testPixelx, testPixely;


	CCoordinateMappingBasics application;
	application.Run(hInstance, nShowCmd);
	vMarkKinectPos = application.getMarkKinectPos();

//	std::shared_ptr<ICalibPro::ICalib> PTest = ICalibPro::CreatePICalib();
//	PTest->Run(hInstance, nShowCmd);
//	vMarkKinectPos = PTest->getMarkKinectPos();


	//标定的顺序对实际点是顺时针方向//
	CDepthBasics application1(vMarkScreenPos, vMarkKinectPos, KinectInstallPattern,h, h1, Radius);
	application1.Run(hInstance, nShowCmd);
  }



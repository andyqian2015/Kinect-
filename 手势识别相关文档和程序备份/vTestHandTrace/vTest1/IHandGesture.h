#pragma once
 #include "ICalib.h"
 #include "Windows.h"
#include <memory>
namespace NAMESPACE_HANDGESTURE
{
	__interface IGetHands {
		void fGetHands(HINSTANCE hInstance, int nCmdShow);
		void _stdcall Test(std::vector<sAction> HandsGesture);
	};
	typedef std::shared_ptr<NAMESPACE_HANDGESTURE::IGetHands> PIGetHands;
	PIGetHands createPIGetHands(HINSTANCE hInstance, int nCmdShow);
} 
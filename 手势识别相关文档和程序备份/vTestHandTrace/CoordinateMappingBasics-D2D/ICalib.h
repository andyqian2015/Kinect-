
#pragma  once
#ifndef  CALIB_H
#define  CALIB_H

#ifdef CALIB_DLL
#define CALIBApi __declspec(dllexport)
#else
#define CALIBApi __declspec(dllimport)
#endif
#include <iostream>
#include "boost/shared_ptr.hpp"
#include "Windows.h"
#include "OpenExr/include/ImathVec.h"
#include "iostream"
#include "vector"
#include "IDLPGesture.h"
#include <functional>
using namespace IMATH_NAMESPACE;

namespace ICalibPro
{
	__interface ICalib {
		int Run(HINSTANCE hInstance, int nCmdShow);
		std::vector<IMATH_NAMESPACE::V3f> getMarkKinectPos();
	};
	typedef std::shared_ptr<ICalibPro::ICalib> PICalib;
	CALIBApi PICalib CreatePICalib();
}


namespace IGetHandGesturePro {

	typedef std::function<void(std::vector<sAction> &HandGesture)> CallBack;

	__interface IGetHandGesture {
		int Run(HINSTANCE hInstance, int nCmdShow);
		void Registry(IGetHandGesturePro::CallBack callback);
		
	};

	typedef std::shared_ptr<IGetHandGesture> PIGetHandGesture;
	CALIBApi PIGetHandGesture createPIGetHandGesture(const std::vector<V3f> & vmarkscreenpos, const std::vector<V3f> & vmarkKinectPos, int KinectInstallPattern, float h, float h1, float Radius);
}



#endif // ! CALIB_H





#pragma once 
#include <functional>
#ifndef ITESTPREINTERFACE_H
#define ITESTPREINTERFACE_H

#ifdef ITESTPREINTERFACE_DLL
#define  ITESTApi __declspec(dllexport)
#else 
#define ITESTApi __declspec(dllimport)
#endif // ITESTPREINTERFACE_DLL


#include "Windows.h"
#include <Memory>
#include "iostream"
#include "vector"

namespace  NAMESPACE_TESTPRE {
	typedef std::function<void(std::vector<int> vec)> CallBack;
	__interface ITestPreInterface {
		int Run(HINSTANCE hInstance, int nCmdShow);
		void Registry(NAMESPACE_TESTPRE::CallBack callback);
	};
	typedef std::shared_ptr<NAMESPACE_TESTPRE::ITestPreInterface> PITestPreInterface;
	ITESTApi PITestPreInterface createPITestPreInterface();
}


namespace NAMESPACE_TESTPRE1
{
	__interface ITestInterfaceADD {
		float add(float a, float b);
	};
	typedef std::shared_ptr<NAMESPACE_TESTPRE1::ITestInterfaceADD> PITestInterface;
	ITESTApi PITestInterface createPITestInterface();
}
#endif

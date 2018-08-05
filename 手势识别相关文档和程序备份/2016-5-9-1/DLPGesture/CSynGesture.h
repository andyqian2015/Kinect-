#pragma once
#ifndef CSYNGESTURE_H
#define CSYNGESTURE_H
#include "dlpgesture\IDLPGesture.h"
using namespace GestureEng;

class CSynGesture : public GestureEng::ISynGesture, public CUnknown {
  public :
	  DECLARE_IUNKNOWN
	  STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void **ppv);
	  CSynGesture();
	  BOOL InitDevice(); //启动设备，启动线程//

	  //获取基本手势初始化，标定数据//
	  BOOL InitGesReg(const std::vector<nvmath::Vec3f> & vMarkScreemPoints, const std::vector<nvmath::Vec3f> & vMarkKinectPoints);
	  BOOL Calib(const std::vector<nvmath::Vec3f> & vMarkScreemPoints, const std::vector<nvmath::Vec2f> &CalibPixelPoints);
//	  BOOL RegistryCallBack(void(*f)(std::vector<sGesture> g));
	  void Registry(mycallbackfunc callbackfunc);

	  BOOL SetEnableGesture(eGestureType gtype, bool bEnable);

private:

	void _stdcall GetGesture(std::vector<sGesture> g);

	PIKinectAction   m_pkinectAction;
	PIMyKinectSensor m_mykiectSensor;
	PIDLPUnknown     m_pmykinectSensor;

	mycallbackfunc   m_func;
	PIEvent          m_PIEvent;
	mycallbackfunc   m_fun;
};


#endif
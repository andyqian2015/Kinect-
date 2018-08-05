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
	  BOOL InitDevice(); //�����豸�������߳�//

	  //��ȡ�������Ƴ�ʼ�����궨����//
	  BOOL InitGesReg(const std::vector<nvmath::Vec3f> & vMarkScreemPoints, const std::vector<nvmath::Vec3f> & vMarkKinectPoints);
	  BOOL Calib(const std::vector<nvmath::Vec3f> & vMarkScreemPoints, const std::vector<nvmath::Vec2f> &CalibPixelPoints);
	  void StartReg();  //�������ƻ�ȡ//
	  void Registry(mycallbackfunc callbackfunc);
	  void SetGestureConnect(PISendSynGesture pGesture);
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
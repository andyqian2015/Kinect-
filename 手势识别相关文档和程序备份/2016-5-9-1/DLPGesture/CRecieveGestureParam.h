#ifndef CRECIEVEGESTUREPARAM_H
#define CRECIEVEGESTUREPARAM_H

#include "GestureParam.h"

class CRecieveGestureParam : public GestureEng::IGestureParam
{
public:
	CRecieveGestureParam();
	~CRecieveGestureParam();
	void						Recognize(const sAction &r);

	bool						SetGestureParam(UINT64 time, float radius,UINT64 time_l);

	bool						SetGestureParamClick(float radius);

	bool						SetGestureParamDClick(UINT64 time, float radius);

	bool						SetGestureParamFlip(double speed, float distance);

	bool						SetGestureParamZoomARoatate(double angle, double proportion);

	bool						SetGestureParamSlideBack(double proportion, float distance, float distancd_back);

	bool						GetChecked(eGestureType type);

	bool						SetDisable(eGestureType type, bool Enable);

	void						GetParamValue();

	void						ResetParam();

	std::vector<sGesture>		GetGesture();

	sGestureParam*							PassBasicParam();
	GestureEng::sGestureParamClick*			PassClickParam();
	GestureEng::sGestureParamDClick*		PassDClickParam();
	GestureEng::sGestureParamFlip*			PassFlipParam();
	GestureEng::sGestureParamSlideBack*		PassBackParam();
	GestureEng::sGestureParamZoomARoatate*	PassZARParam();

private:
	std::vector<sGesture>					m_g;
	PIGesture								gesture;
	sGestureParam							*m_basicparam;
	GestureEng::sGestureParamClick			*m_clickparam;
	GestureEng::sGestureParamDClick			*m_dclickparam;
	GestureEng::sGestureParamFlip			*m_flipparam;
	GestureEng::sGestureParamZoomARoatate	*m_zarparam;
	GestureEng::sGestureParamSlideBack		*m_slidebackparam;
};

	GestureEng::PIGestureParam CreateParamPointer();
#endif
//GestureEngine:ȷ����˫���󵥻�
#ifndef CGSCLICK_H
#define CGSCLICK_H

#include "InnerStructure.h"

class CGSClick : public inner :: IGestureEngine
{
public:
	bool IsThisGst();

private:
	void SetFlag();

	inner :: GSTFLAG gstflag;
	sGesture overflag;
	//eGestureType type;
	sAction	 q;		//����²㴫��������
	SHANDGST q1;	//��hand_1st���ص�����
	SHANDGST q2;	//��hand_2nd���ص�����
	SHANDGST sav;	//�ݴ��һ�ε���������
};
#endif
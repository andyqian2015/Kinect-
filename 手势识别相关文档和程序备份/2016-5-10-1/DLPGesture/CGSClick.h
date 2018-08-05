//GestureEngine:确认无双击后单击
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
	sAction	 q;		//存从下层传进的数据
	SHANDGST q1;	//存hand_1st返回的数据
	SHANDGST q2;	//存hand_2nd返回的数据
	SHANDGST sav;	//暂存第一次单击的数据
};
#endif
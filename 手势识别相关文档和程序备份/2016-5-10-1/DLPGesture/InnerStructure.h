#pragma once


#ifndef INNERSTRUCTURE_H
#define INNERSTRUCTURE_H


#include "dlpgesture/IDLPGesture.h"


namespace GestureEng
{

// 	static UINT movelimit;
// 
// 	extern	UINT64	 T_MAX;// = 999;
// 	extern	UINT64	 T_MIN;// = 3;
// 	extern	UINT64	 DCLICK_T;// = 1000;
// 	extern	double	 v;// = 1.0;
// 	extern	float	 distance;// = 600.0;
// 	extern	float	 back_confirm;// = 100.0;

// 	struct ACTION
// 	{
// 		static int					ID;
// 		static UINT64				t;    //时间戳  毫秒
// 		static eActionState			st;   //状态
// 		static Vec3<float>			pos;
// 	};

	struct DATAUSF
	{
		DATAUSF() :FLAG(false){}
		bool		FLAG;		//标志是否接受数据
		sAction		usfdata;	
	};

	struct HANDFLAG
	{
		HANDFLAG() :flag_down_trig(false), flag_move_trig(false),
			flag_up_trig(false){}
		//bool flag_id_trig;
		bool		flag_down_trig;
		bool		flag_move_trig;
		bool		flag_up_trig;

	};

	enum sHand{		//单手动作
		Neither,	//初始化值
		Click,		//单手点击
		Moveto,		//单手移动
		Lpush		//单手长按
	};

	struct SHANDGST	//单手传出数据结构
	{
		SHANDGST() :overflag(false), st(Neither),t_ori(0),t(0){}
		bool			overflag;
		sHand			st;	
		UINT64			t_ori;
		UINT64			t;
		GestureEng::COORD3		pos_ori;
		GestureEng::COORD3		pos_fin;
	};

	struct GSTFLAG	// 双手标志
	{
		GSTFLAG() :dhand_flag(false), click_flag(false),
		push_flag(false), move_flag(false), over_flag(false){}
		bool			dhand_flag;
		bool			click_flag;
		bool			push_flag;
		bool			move_flag;
		bool			over_flag;
	};


}
#endif
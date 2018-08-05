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
// 		static UINT64				t;    //ʱ���  ����
// 		static eActionState			st;   //״̬
// 		static Vec3<float>			pos;
// 	};

	struct DATAUSF
	{
		bool		FLAG;		//��־�Ƿ��������
		sAction		usfdata;	
	};

	struct HANDFLAG
	{
		//bool flag_id_trig;
		bool		flag_down_trig;
		bool		flag_move_trig;
		bool		flag_up_trig;

	};

	enum sHand{		//���ֶ���
		Neither,	//��ʼ��ֵ
		Click,		//���ֵ��
		Moveto,		//�����ƶ�
		Lpush		//���ֳ���
	};

	struct SHANDGST	//���ִ������ݽṹ
	{
		bool			overflag;
		sHand			st;	
		UINT64			t_ori;
		UINT64			t;
		COORD3		    pos_ori;
		COORD3		    pos_fin;
	};

	struct GSTFLAG	// ˫�ֱ�־
	{
		bool			dhand_flag;
		bool			click_flag;
		bool			push_flag;
		bool			move_flag;
		bool			over_flag;
	};


}
#endif
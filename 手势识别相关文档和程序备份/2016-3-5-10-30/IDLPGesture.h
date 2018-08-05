#pragma once
#ifndef GDATASTRUCTER_H
#define GDATASTRUCTER_H

#ifdef GSTRCN_DLL
#define GstRcnApi __declspec(dllexport)
#else
#define GstRcnApi __declspec(dllimport)
#endif

//����ʶ�����ӳ����ݱ�׼




#include <memory>



enum eActionState{
	None,   
	Down,
	Up,
	Move
};//�Ӵ�״̬

namespace GestureEng
{
	struct COORDINATE3
	{
		float x;
		float y;
		float z;
	};

	struct COORDINATE2
	{
		float x;
		float y;
	};
}

struct sAction //����
{
	int							ID;
	UINT64						t;    //ʱ���  ����
	eActionState				st;   //״̬
	//Vec3<float>				pos;  //��Ļ����
	GestureEng::COORDINATE3   	pos;  //��Ļ����
	GestureEng::COORDINATE3     worldPos;//Kinect����//
	GestureEng::COORDINATE3   	critpos;
};


enum eSensorImage
{
	eSI_None,
	eSI_Depth,  
	eSI_RGB,
	eSI_Hand,
	eSI_Box,
};


__interface IDepthSensor
{
	//���ش�������Ϣ
	//const sDepthSensorInfo& GetName(); 

	//��ʾ������ͼ�� width��height��ʾ����ĳߴ�
	void		ShowImage(HDC hDC,int width,int height,eSensorImage imgType);

	//���ظ��������ֽ���  pDstBuff�������sDepthSensorInfo�Ĺ��
	int			CopyImage(BYTE* pDstBuff,int nDstPitch,eSensorImage imgType );

	int			SaveImage(const char* filename);


	//��λ������,����һ�������ʵ�ռ����꣨������ʵ����ϵԭ�㣩�ʹ�����ͼ�����꣬���ش���������ʵ���꣨���ο���
	//ע�⣺��װʱ��������ֱ���£�����������ϵ����������ϵ��x�����y���������᷽��һ��
	//const Point3f&	LocatePosition(const Point3f& pt_image,const Point3f& pt_real);
	//nPoints:��λ��������С��2��scr����Ļ���꣬real��������ʶ�������ʵ�ռ�����
	void		SetCoordinateTrans(int nPoints,GestureEng::COORDINATE2* scr,GestureEng::COORDINATE3* real);

};
typedef std::shared_ptr<IDepthSensor> PIDepthSensor;


__interface IGesutreEvent{
	void		OnGestureData(const sAction& data );
};
typedef std::shared_ptr<IGesutreEvent> PIGesutreEvent;


__interface IGestrueSource{
	void		ConnectEvent(PIGesutreEvent pEvent,bool bConnect=true );
	void		Start();	//��������
	void		Stop();	//��������
};
typedef std::shared_ptr<IGestrueSource> PIGestrueSource;



enum eGestureType
{
	GMoveTo,		//0�ƶ�					�����¼� �ƶ�����
	GClick,			//1���					�����¼� ����󴥷� ���º���ʱ����С��ĳֵ λ����ĳ������
	GLongPush,      //2����					�����¼� ���º����ʱ�����ĳֵ �����ƶ� ����󲻴��������¼�

	GNone,			//3���ƽ�����־			�������ƺ�Ĵ���OVER��־�����ݵ���󴥷�
	GSingleClick,	//4����					�����һ��ʱ�������޵������ʱ���� һ�ε�������һ�ε��
	GDoubleClick,	//5˫��					���ε��ʱ����С��ĳֵ һ��˫�����̻ᴥ�����ε��
	GFlipRight,     //6���һ���-��ҳ			���һ����㹻�����ӽ�������ʼ�Խϴ��ٶȻ��� ����󴥷�   
	GFlipLeft,      //7���󻬶�-��ҳ			���󻬶��㹻�����ӽ�������ʼ�Խϴ��ٶȻ��� ����󴥷�
	GSwithHori,     //8ˮƽ���ػ���-�л�ģʽ	��⵽�ػ��㹻����󴥷� һ�λػ����������ƶ�
	GFlipUp,	    //9���ϻ���-��ҳ			���ϻ����㹻�����ӽ�������ʼ�Խϴ��ٶȻ��� ����󴥷�
	GFlipDown,	    //10���»���-��ҳ			���»����㹻�����ӽ�������ʼ�Խϴ��ٶȻ��� ����󴥷�
	GSwithVert,     //11��ֱ���ػ���-�л�ģʽ	��⵽�ػ��㹻����󴥷� һ�λػ����������ƶ�
	GDrag,          //12�϶�					LongPush���ƶ����� ����󲻴��������¼�
	

	DualHand,		//13
	GZoom,           //14����					ֻ��˫�� 
	GRotate,         //15��ת					ֻ��˫��

};//����ʶ������


struct sGesture
{
	eGestureType		gtype;		//��������  eGesture����ָ����λ
	uint64_t			t;			//ʱ���

	//�ƶ�
	GestureEng::COORDINATE3			pos;	    //�ֵĵ�ǰλ�� 

	//����
	double				fScale;				//����ϵ��  ��ʼ����Ϊ1

	//��ת
	double				fScreenRotateAngle; //��ת����  ��ʼΪ0 ��ʱ��Ϊ�� ˳ʱ��Ϊ��
	GestureEng::COORDINATE3			fScreenPivot;       //��ת����  2���ֵ�����
};

struct sGestureParam
{
	sGestureParam() :type(DualHand), time_max(120000), radius_move(10), time_longpush(1000){};
	//virtual void SetParam(UINT64 time,float radius,UINT64 time_l);
	eGestureType		type;
	UINT64				time_max;//�����ж�ʱ�� 
	float				radius_move;//�˷�Χ����Ϊû���ƶ�
	UINT64				time_longpush;
};

//3������ʶ��ӿ�
__interface IGesture
{
	

	//�趨�ж��Ƿ��ƶ��ķ�Χ 
	void					SetMoveLimit(const sGestureParam *parm);

	//�����������ԣ��������ö����Ҫʶ������ƣ����������Զ�����
	int						SetGestureMode( const sGestureParam *gc );

	//����ʶ������ƣ�����ͬʱʶ��������
	void					EnableGesture( eGestureType gtype ,bool bEnable );

	//��λ�������õ���ʼ״̬
	void					Reset();

	//ĳ���˵��������ݴ��룬�õ��ض�����״̬��ͨ����ʼ�׶Σ�����min_timeʱ���ڣ�����������ƣ�֮�����������������״̬
	void					OnAction(const sAction &a,std::vector<sGesture> &g);//������յ��ֶ�������ô˽ӿں���

	//�����ȡ����ֵ���������ý���Ĭ��ֵ��ʵ�ʲ���һ��
	void*					GetParamValue(const int);

	bool					GetChecked(const UINT i);
};
typedef std::shared_ptr<IGesture> PIGesture;

//����ʶ��Engine
__interface IGestureEngine
{
	int						Initialize();//��ʼ��HandTrace�����ش�����������
	void					Close();	//�رղ��ͷ���Դ

	UINT					GetSensorCount();
	PIDepthSensor			GetSensor(UINT idx);

	//��ȡ���ƶ������ٽӿ�
	PIGestrueSource			GetHandTrace();
};
typedef std::shared_ptr<IGestureEngine> PIGestureEngine;

GstRcnApi PIGestureEngine   CreateGestureEngine();
GstRcnApi PIGesture			CreateGesture();




#endif 
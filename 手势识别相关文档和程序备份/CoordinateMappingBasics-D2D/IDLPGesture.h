#pragma once
#ifndef GDATASTRUCTER_H
#define GDATASTRUCTER_H

#ifdef GSTRCN_DLL
#define GstRcnApi __declspec(dllexport)
#else
#define GstRcnApi __declspec(dllimport)
#endif

//手势识别接入接出数据标准




#include <memory>



enum eActionState{
	None,   
	Down,
	Up,
	Move
};//接触状态

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

struct sAction //动作
{
	int							ID;
	UINT64						t;    //时间戳  毫秒
	eActionState				st;   //状态
	//Vec3<float>				pos;  //屏幕坐标
	GestureEng::COORDINATE3   	pos;  //屏幕坐标
	GestureEng::COORDINATE3     worldPos;//Kinect坐标//
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
	//返回传感器信息
	//const sDepthSensorInfo& GetName(); 

	//显示传感器图像 width，height显示窗体的尺寸
	void		ShowImage(HDC hDC,int width,int height,eSensorImage imgType);

	//返回复制数据字节数  pDstBuff必须符合sDepthSensorInfo的规格
	int			CopyImage(BYTE* pDstBuff,int nDstPitch,eSensorImage imgType );

	int			SaveImage(const char* filename);


	//定位传感器,传入一个点的真实空间坐标（比如真实坐标系原点）和传感器图像坐标，返回传感器的真实坐标（供参考）
	//注意：安装时传感器垂直向下，传感器坐标系和世界坐标系的x方向和y方向坐标轴方向一致
	//const Point3f&	LocatePosition(const Point3f& pt_image,const Point3f& pt_real);
	//nPoints:定位点数，最小是2，scr：屏幕坐标，real：传感器识别出的真实空间坐标
	void		SetCoordinateTrans(int nPoints,GestureEng::COORDINATE2* scr,GestureEng::COORDINATE3* real);

};
typedef std::shared_ptr<IDepthSensor> PIDepthSensor;


__interface IGesutreEvent{
	void		OnGestureData(const sAction& data );
};
typedef std::shared_ptr<IGesutreEvent> PIGesutreEvent;


__interface IGestrueSource{
	void		ConnectEvent(PIGesutreEvent pEvent,bool bConnect=true );
	void		Start();	//启动跟踪
	void		Stop();	//结束跟踪
};
typedef std::shared_ptr<IGestrueSource> PIGestrueSource;



enum eGestureType
{
	GMoveTo,		//0移动					基础事件 移动触发
	GClick,			//1点击					基础事件 弹起后触发 按下后弹起时间间隔小于某值 位置在某区域内
	GLongPush,      //2长按					基础事件 按下后持续时间大于某值 不能移动 弹起后不触发其它事件

	GNone,			//3手势结束标志			完整手势后的带有OVER标志的数据到达后触发
	GSingleClick,	//4单击					点击后一段时间间隔内无点击操作时触发 一次单击触发一次点击
	GDoubleClick,	//5双击					两次点击时间间隔小于某值 一次双击过程会触发两次点击
	GFlipRight,     //6向右滑动-翻页			向右滑动足够距离或从进入区域开始以较大速度滑过 弹起后触发   
	GFlipLeft,      //7向左滑动-翻页			向左滑动足够距离或从进入区域开始以较大速度滑过 弹起后触发
	GSwithHori,     //8水平来回滑动-切换模式	检测到回滑足够距离后触发 一次回滑触发数次移动
	GFlipUp,	    //9向上滑动-翻页			向上滑动足够距离或从进入区域开始以较大速度滑过 弹起后触发
	GFlipDown,	    //10向下滑动-翻页			向下滑动足够距离或从进入区域开始以较大速度滑过 弹起后触发
	GSwithVert,     //11垂直来回滑动-切换模式	检测到回滑足够距离后触发 一次回滑触发数次移动
	GDrag,          //12拖动					LongPush后移动触发 弹起后不触发其它事件
	

	DualHand,		//13
	GZoom,           //14缩放					只能双手 
	GRotate,         //15旋转					只能双手

};//手势识别种类


struct sGesture
{
	eGestureType		gtype;		//手势类型  eGesture符号指定的位
	uint64_t			t;			//时间戳

	//移动
	GestureEng::COORDINATE3			pos;	    //手的当前位置 

	//缩放
	double				fScale;				//缩放系数  初始缩放为1

	//旋转
	double				fScreenRotateAngle; //旋转度数  初始为0 逆时钟为正 顺时钟为负
	GestureEng::COORDINATE3			fScreenPivot;       //旋转中心  2个手的中心
};

struct sGestureParam
{
	sGestureParam() :type(DualHand), time_max(120000), radius_move(10), time_longpush(1000){};
	//virtual void SetParam(UINT64 time,float radius,UINT64 time_l);
	eGestureType		type;
	UINT64				time_max;//故障判定时间 
	float				radius_move;//此范围内视为没有移动
	UINT64				time_longpush;
};

//3、手势识别接口
__interface IGesture
{
	

	//设定判断是否移动的范围 
	void					SetMoveLimit(const sGestureParam *parm);

	//设置手势属性，可以设置多个需要识别的手势，跟踪数据自动清零
	int						SetGestureMode( const sGestureParam *gc );

	//允许识别的手势，可以同时识别多个手势
	void					EnableGesture( eGestureType gtype ,bool bEnable );

	//复位手势设置到初始状态
	void					Reset();

	//某个人的人手数据传入，得到特定手势状态，通常起始阶段（手势min_time时间内）不会产生手势，之后可能连续生成手势状态
	void					OnAction(const sAction &a,std::vector<sGesture> &g);//物体接收到手动作后调用此接口函数

	//界面获取参数值，保持设置界面默认值和实际参数一致
	void*					GetParamValue(const int);

	bool					GetChecked(const UINT i);
};
typedef std::shared_ptr<IGesture> PIGesture;

//手势识别Engine
__interface IGestureEngine
{
	int						Initialize();//初始化HandTrace，返回传感器的数量
	void					Close();	//关闭并释放资源

	UINT					GetSensorCount();
	PIDepthSensor			GetSensor(UINT idx);

	//获取手势动作跟踪接口
	PIGestrueSource			GetHandTrace();
};
typedef std::shared_ptr<IGestureEngine> PIGestureEngine;

GstRcnApi PIGestureEngine   CreateGestureEngine();
GstRcnApi PIGesture			CreateGesture();




#endif 
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

//V2
#include "nvmath\Vecnt.h"
#include <iostream>
#include <vector>
#include "DLPUtils/DLPUnknown.h"
#include "DLPUtils/combase.h"
//V2

//V2
#include "OpenExr/include/ImathVec.h"
#include "OpenExr/include/ImathLine.h"
#include "OpenExr/include/ImathLineAlgo.h"
#include "OpenExr/include/ImathPlane.h"
#include "OpenExr/include/ImathQuat.h"
using namespace IMATH_NAMESPACE;
//V2



namespace GestureEng
{


	struct COORD3
	{
		COORD3(float x0 = 0, float y0 = 0, float z0 = 0) :x(x0), y(y0), z(z0) {}
		float x;
		float y;
		float z;
	};

	struct COORD2
	{
		COORD2() : x(0), y(0) {}
		float x;
		float y;
	};


	struct SGestureRegInitParam
	{
		SGestureRegInitParam() :h(500), h1(50), Radius(30) {}
		int h;                    //判断可识别区域离屏距离//
		int h1;                   //将离屏的深度置0的深度//
		float Radius;             //判断手势运动的阈值//
	};

	enum eActionState{
		None,
		Down,
		Up,
		Move
	};//接触状态


	enum eKinectImage{
		KI_None,
		KI_COLOR,
		KI_Depth,
		KI_IR,
	};

	struct sAction //动作
	{
		int							ID;
		UINT64						t;    //时间戳  毫秒
		eActionState				st;   //状态
		COORD3   	                pos;  //屏幕坐标
	};



	//V2
	struct sAction1 :public  sAction //动作
	{
		nvmath::Vec3f               worldPos;  //为手的轮廓尖端在Kinect坐标系下的坐标//
		COORD3   	                critpos;
	};



	enum eInstallMode {
		Forward = 0,
		BackWard,
	};

	struct sColorDiffIndex {
		sColorDiffIndex() :nRedDiffIndex(40), nGreenDiffIndex(40), nBlueDiffIndex(40) {}
		int nRedDiffIndex;
		int nGreenDiffIndex;
		int nBlueDiffIndex;
	};


	enum eSensorImage
	{
		eSI_None,
		eSI_Depth,
		eSI_RGB,
		eSI_Hand,
		eSI_Box,
	};



	enum eGestureType
	{
		GMoveTo,		//0移动					基础事件 移动触发
		GClick,			//1点击					基础事件 弹起后触发 按下后弹起时间间隔小于某值 位置在某区域内
		GLongPush,      //2长按					基础事件 按下后持续时间大于某值 不能移动 弹起后不触发其它事件

		GSingleClick,	//3单击					点击后一段时间间隔内无点击操作时触发 一次单击触发一次点击
		GDoubleClick,	//4双击					两次点击时间间隔小于某值 一次双击过程会触发两次点击
		GFlipRight,     //5向右滑动-翻页			向右滑动足够距离或从进入区域开始以较大速度滑过 弹起后触发   
		GFlipLeft,      //6向左滑动-翻页			向左滑动足够距离或从进入区域开始以较大速度滑过 弹起后触发
		GSwithHori,     //7水平来回滑动-切换模式	检测到回滑足够距离后触发 一次回滑触发数次移动
		GFlipUp,	    //8向上滑动-翻页			向上滑动足够距离或从进入区域开始以较大速度滑过 弹起后触发
		GFlipDown,	    //9向下滑动-翻页			向下滑动足够距离或从进入区域开始以较大速度滑过 弹起后触发
		GSwithVert,     //10垂直来回滑动-切换模式	检测到回滑足够距离后触发 一次回滑触发数次移动
		GDrag,          //11拖动					LongPush后移动触发 弹起后不触发其它事件


		DualHand,		//12
		GZoom,          //13缩放					只能双手 
		GRotate,		//14旋转					只能双手

		GNone,			//15手势结束标志			完整手势后的带有OVER标志的数据到达后触发      

		Gesture_COUNT

	};//手势识别种类


	struct sGesture
	{
		sGesture() :gtype(DualHand), t(0), fScale(1), fScreenRotateAngle(0){}
		eGestureType	gtype;		//手势类型  eGesture符号指定的位
		UINT64			t;			//时间戳

		//移动
		GestureEng::COORD3			pos;	    //手的当前位置 

		//缩放
		double				fScale;				//缩放系数  初始缩放为1

		//旋转
		double				fScreenRotateAngle; //旋转度数  初始为0 逆时钟为正 顺时钟为负
		GestureEng::COORD3			fScreenPivot;       //旋转中心  2个手的中心
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

	__interface IDepthSensor
	{
		//返回传感器信息
		//const sDepthSensorInfo& GetName(); 

		//显示传感器图像 width，height显示窗体的尺寸
		void		ShowImage(HDC hDC, int width, int height, eSensorImage imgType);

		//返回复制数据字节数  pDstBuff必须符合sDepthSensorInfo的规格
		int			CopyImage(BYTE* pDstBuff, int nDstPitch, eSensorImage imgType);

		int			SaveImage(const char* filename);


		//定位传感器,传入一个点的真实空间坐标（比如真实坐标系原点）和传感器图像坐标，返回传感器的真实坐标（供参考）
		//注意：安装时传感器垂直向下，传感器坐标系和世界坐标系的x方向和y方向坐标轴方向一致
		//const Point3f&	LocatePosition(const Point3f& pt_image,const Point3f& pt_real);
		//nPoints:定位点数，最小是2，scr：屏幕坐标，real：传感器识别出的真实空间坐标
		void		SetCoordinateTrans(int nPoints, GestureEng::COORD2* scr, GestureEng::COORD3* real);

	};
	typedef std::shared_ptr<IDepthSensor> PIDepthSensor;


	__interface IGesutreEvent{
		void		OnGestureData(const sAction& data);
	};
	typedef std::shared_ptr<IGesutreEvent> PIGesutreEvent;


	__interface IGestrueSource{
		void		ConnectEvent(PIGesutreEvent pEvent, bool bConnect = true);
		void		Start();	//启动跟踪
		void		Stop();	//结束跟踪
	};
	typedef std::shared_ptr<IGestrueSource> PIGestrueSource;

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


	//3、手势识别接口
	__interface IGesture
	{

		//设置手势属性，可以设置多个需要识别的手势，跟踪数据自动清零
		int						SetGestureParam(const sGestureParam *gc);

		//允许识别的手势，可以同时识别多个手势
		void					EnableGesture(eGestureType gtype, bool bEnable);

		//复位手势设置到初始状态
		void					Reset();

		//某个人的人手数据传入，得到特定手势状态，通常起始阶段（手势min_time时间内）不会产生手势，之后可能连续生成手势状态
		void					OnAction(const std::vector<sAction> &a, std::vector<sGesture> &g);//物体接收到手动作后调用此接口函数

		//界面获取参数值，保持设置界面默认值和实际参数一致
		const sGestureParam*	GetParamValue(eGestureType type);

		bool					GetChecked(eGestureType type);
	};
	typedef std::shared_ptr<IGesture> PIGesture;

	__interface __declspec(uuid("{3F8115EB-5DE2-4023-B644-390F38336CF6}"))  IKinectImage :public IDLPUnknown
	{
		void OnImage(const void* pBuff, int nW, int nH, int nPitch, eKinectImage nImageType, UINT uTimestamp);

	};
	typedef NSINSIVE_PTR::intrusive_ptr<IKinectImage> PIKinectImage;



	__interface __declspec(uuid("{426E3ADB-646B-4A34-B430-831BCEBD3191}"))IKinectSkeleton :public IDLPUnknown
	{
		void OnSkeleton();
	};
	typedef NSINSIVE_PTR::intrusive_ptr<IKinectSkeleton> PIKinectSkeleton;


	__interface __declspec(uuid("{A9B90F38-AEEC-4500-BA19-640369E3993E}")) IMyKinectSensor :public IDLPUnknown{
		UINT GetCount();
		BOOL Init(UINT uIndex = 0);

		//pRCV:IKinectSkeleton, IKinectImage//
		BOOL Connect(PIDLPUnknown pRcv, BOOL bConnect, eKinectImage ImageType);
		void Close();
	};
	typedef NSINSIVE_PTR::intrusive_ptr<IMyKinectSensor> PIMyKinectSensor;

	//color-depth transform//
	__interface __declspec(uuid("{EBC2E474-595A-47D0-8A97-A7CBEB3F6F12}")) IImgTransform : public IDLPUnknown {
		BOOL ColorPixelToDepthPos(const void * depthBuffer, int depthwidth, int depthheight, const void* colorBuffer, int colorwidth, int colorheight, const sColorDiffIndex &colorDiffIndex, std::vector<nvmath::Vec2f> &CalibPixelPoints, std::vector<nvmath::Vec3f> &vMarkKinectPos);
	};
	typedef NSINSIVE_PTR::intrusive_ptr<IImgTransform> PIImgTransform;


	//手势识别动作
	__interface __declspec(uuid("{2A78DCDA-4D63-4EEA-B614-56E739F7C167}")) IKinectActionEvent :public IDLPUnknown{
		void OnAction(const std::vector<sAction1>&  sAction);
//		void  GetHands(std::vector<sAction1> & sHandActions);
	};
	typedef NSINSIVE_PTR::intrusive_ptr<IKinectActionEvent> PIKinectActionEvent;


	//query支持IKinectDataEvent
	__interface __declspec(uuid("{EED3FE89-2B27-48E9-B0F0-8F31963ADA18}")) IKinectAction :public IDLPUnknown
	{

		//参数设置
//		void SetColorDiffIndex(const sColorDiffIndex &colordiffindex);
//		void SetKinectInStallMode(eInstallMode InstallMode);
		BOOL SetRegParam(const SGestureRegInitParam &sRegParam);

		BOOL Init(PIDLPUnknown pKinectSensor, const std::vector<nvmath::Vec3f> & vMarkScreemPoints, const std::vector<nvmath::Vec3f> & vMarkKinectPos);

		BOOL Calib(const std::vector<nvmath::Vec3f> & vMarkScreemPoints, const std::vector<nvmath::Vec2f> &CalibPixelPoints);

		void GetCalibParam(std::vector<nvmath::Vec3f> & vMarkScreemPoints, std::vector<nvmath::Vec3f> & vMarkKinectPoints);

		//pRcv: IKinectActionEvent
		BOOL Connect(PIDLPUnknown pKinectActionEvent, bool bConnect);
	};
	typedef NSINSIVE_PTR::intrusive_ptr<IKinectAction> PIKinectAction;

	//IEvent:用于和Gesture连接以及和KinectActionEvent进行连接//
	typedef boost::function<void(std::vector<sGesture>)> mycallbackfunc;
	__interface __declspec(uuid("{F053564F-ABCD-4208-91F6-2FABD73FD840}")) IEvent : public IDLPUnknown
	{

		BOOL Init(PIDLPUnknown pKinectAction);   //初始化m_PIKinectAction//
		BOOL SetEnableGesture(GestureEng::eGestureType gtype, bool bEnable); //设置允许的手势//
//		void GetGesture();                               //获取手势//

		void Registry(mycallbackfunc callbackfunc);

//		void Test(GestureEng::sGesture &s);

	};
	typedef NSINSIVE_PTR::intrusive_ptr<IEvent>  PIEvent;


	__interface __declspec(uuid("{C18BE961-7534-4003-A0FC-73ACD80A903B}")) ISynGesture : public IDLPUnknown
	{
		BOOL InitDevice(); //启动设备，启动线程//
		BOOL InitGesReg(const std::vector<nvmath::Vec3f> & vMarkScreemPoints, const std::vector<nvmath::Vec3f> & vMarkKinectPoints);
		BOOL Calib(const std::vector<nvmath::Vec3f> & vMarkScreemPoints, const std::vector<nvmath::Vec2f> &CalibPixelPoints);
		BOOL SetEnableGesture(eGestureType gtype, bool bEnable);
		void Registry(mycallbackfunc callbackfunc);
	};
	typedef NSINSIVE_PTR::intrusive_ptr<ISynGesture> PISynGesture;

	//-----------------------------------------------------------
	//3维抓取
	//query支持IKinectDataEvent
	__interface IKinectGrap :public IDLPUnknown{
		//参数设置
	};
	typedef NSINSIVE_PTR::intrusive_ptr<IKinectGrap> PIKinectGrap;


	__interface __declspec(uuid("{9FF128E8-524C-4BDF-B513-E8EFB731BDB7}")) IHandTrace :public IDLPUnknown
	{
		BOOL InitData(const int &ImgWidth, const int &ImgHeight, const std::vector<nvmath::Vec3f> & vmarkscreenpos, const std::vector<nvmath::Vec3f> & vmarkKinectPos, eInstallMode installMode, float h, float h1, float Radius);
		BOOL GetHandGesture(const void* pBuffer, int Imagewidth, int Imageheight, std::vector<sAction1> &handAction, INT64 utimestamp);
	};
	typedef NSINSIVE_PTR::intrusive_ptr<IHandTrace> PIHandTrace;


	//test Calib//
	__interface __declspec(uuid("{CEA12F4A-E574-457A-8CF0-98F353E25C59}")) ICalib : public IDLPUnknown
	{
		//	void GetRegZone(std::vector<nvmath::Vec3f> & vPixelPos);

		//test for use//
		void GetAllCounter(const int &colorwidth, const int & colorheight, const void* pColorBuffer);
		//test for use//


		//	void GetRegZone(std::vector<cv::Point> contours, std::vector<nvmath::Vec2f> & vPixelPos);
		void SetScreenPos(const std::vector<nvmath::Vec3f> & vRegZonePixel, const std::vector<nvmath::Vec3f> & vPixelPos, std::vector<nvmath::Vec3f> & vKinectPos);
		void SetScreenPos_1(const std::vector<nvmath::Vec3f> &vpixelKinectPos, const std::vector<nvmath::Vec3f>& vKinectPos);
	};

	typedef NSINSIVE_PTR::intrusive_ptr<ICalib> PICalib;
	//test Calib//

	//test for use//

	__interface IMathBase  {
	public:
		void  getKinectPos(const V3f & pos, V3f & Point, int DepthWidth, int DepthHeight);
		void  getPixelPos(const V3f & pos, V3f &pixelPoint, int DepthWidth, int DepthHeight);
		void  getProjectPos(const Plane3f &ScreenPlane, const V3f & sKinectPos, V3f & vProjectPoint);
	};
	typedef std::shared_ptr<IMathBase> PIMathBase;
	PIMathBase createPIMathBase();

	//test for use//

	GstRcnApi PIDLPUnknown CreateHandObj(REFIID objClassID);
//V2


	GstRcnApi PIGestureEngine   CreateGestureEngine();
	GstRcnApi PIGesture			CreateGesture();

}


#endif 
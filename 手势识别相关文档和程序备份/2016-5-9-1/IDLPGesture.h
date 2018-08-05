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
		int h;                    //�жϿ�ʶ��������������//
		int h1;                   //�������������0�����//
		float Radius;             //�ж������˶�����ֵ//
	};

	enum eActionState{
		None,
		Down,
		Up,
		Move
	};//�Ӵ�״̬


	enum eKinectImage{
		KI_None,
		KI_COLOR,
		KI_Depth,
		KI_IR,
	};

	struct sAction //����
	{
		int							ID;
		UINT64						t;    //ʱ���  ����
		eActionState				st;   //״̬
		COORD3   	                pos;  //��Ļ����
	};



	//V2
	struct sAction1 :public  sAction //����
	{
		nvmath::Vec3f               worldPos;  //Ϊ�ֵ����������Kinect����ϵ�µ�����//
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
		GMoveTo,		//0�ƶ�					�����¼� �ƶ�����
		GClick,			//1���					�����¼� ����󴥷� ���º���ʱ����С��ĳֵ λ����ĳ������
		GLongPush,      //2����					�����¼� ���º����ʱ�����ĳֵ �����ƶ� ����󲻴��������¼�

		GSingleClick,	//3����					�����һ��ʱ�������޵������ʱ���� һ�ε�������һ�ε��
		GDoubleClick,	//4˫��					���ε��ʱ����С��ĳֵ һ��˫�����̻ᴥ�����ε��
		GFlipRight,     //5���һ���-��ҳ			���һ����㹻�����ӽ�������ʼ�Խϴ��ٶȻ��� ����󴥷�   
		GFlipLeft,      //6���󻬶�-��ҳ			���󻬶��㹻�����ӽ�������ʼ�Խϴ��ٶȻ��� ����󴥷�
		GSwithHori,     //7ˮƽ���ػ���-�л�ģʽ	��⵽�ػ��㹻����󴥷� һ�λػ����������ƶ�
		GFlipUp,	    //8���ϻ���-��ҳ			���ϻ����㹻�����ӽ�������ʼ�Խϴ��ٶȻ��� ����󴥷�
		GFlipDown,	    //9���»���-��ҳ			���»����㹻�����ӽ�������ʼ�Խϴ��ٶȻ��� ����󴥷�
		GSwithVert,     //10��ֱ���ػ���-�л�ģʽ	��⵽�ػ��㹻����󴥷� һ�λػ����������ƶ�
		GDrag,          //11�϶�					LongPush���ƶ����� ����󲻴��������¼�


		DualHand,		//12
		GZoom,          //13����					ֻ��˫�� 
		GRotate,		//14��ת					ֻ��˫��

		GNone,			//15���ƽ�����־			�������ƺ�Ĵ���OVER��־�����ݵ���󴥷�      

		Gesture_COUNT

	};//����ʶ������


	struct sGesture
	{
		sGesture() :gtype(DualHand), t(0), fScale(1), fScreenRotateAngle(0){}
		eGestureType	gtype;		//��������  eGesture����ָ����λ
		UINT64			t;			//ʱ���

		//�ƶ�
		GestureEng::COORD3			pos;	    //�ֵĵ�ǰλ�� 

		//����
		double				fScale;				//����ϵ��  ��ʼ����Ϊ1

		//��ת
		double				fScreenRotateAngle; //��ת����  ��ʼΪ0 ��ʱ��Ϊ�� ˳ʱ��Ϊ��
		GestureEng::COORD3			fScreenPivot;       //��ת����  2���ֵ�����
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

	__interface IDepthSensor
	{
		//���ش�������Ϣ
		//const sDepthSensorInfo& GetName(); 

		//��ʾ������ͼ�� width��height��ʾ����ĳߴ�
		void		ShowImage(HDC hDC, int width, int height, eSensorImage imgType);

		//���ظ��������ֽ���  pDstBuff�������sDepthSensorInfo�Ĺ��
		int			CopyImage(BYTE* pDstBuff, int nDstPitch, eSensorImage imgType);

		int			SaveImage(const char* filename);


		//��λ������,����һ�������ʵ�ռ����꣨������ʵ����ϵԭ�㣩�ʹ�����ͼ�����꣬���ش���������ʵ���꣨���ο���
		//ע�⣺��װʱ��������ֱ���£�����������ϵ����������ϵ��x�����y���������᷽��һ��
		//const Point3f&	LocatePosition(const Point3f& pt_image,const Point3f& pt_real);
		//nPoints:��λ��������С��2��scr����Ļ���꣬real��������ʶ�������ʵ�ռ�����
		void		SetCoordinateTrans(int nPoints, GestureEng::COORD2* scr, GestureEng::COORD3* real);

	};
	typedef std::shared_ptr<IDepthSensor> PIDepthSensor;


	__interface IGesutreEvent{
		void		OnGestureData(const sAction& data);
	};
	typedef std::shared_ptr<IGesutreEvent> PIGesutreEvent;


	__interface IGestrueSource{
		void		ConnectEvent(PIGesutreEvent pEvent, bool bConnect = true);
		void		Start();	//��������
		void		Stop();	//��������
	};
	typedef std::shared_ptr<IGestrueSource> PIGestrueSource;

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


	//3������ʶ��ӿ�
	__interface IGesture
	{

		//�����������ԣ��������ö����Ҫʶ������ƣ����������Զ�����
		int						SetGestureParam(const sGestureParam *gc);

		//����ʶ������ƣ�����ͬʱʶ��������
		void					EnableGesture(eGestureType gtype, bool bEnable);

		//��λ�������õ���ʼ״̬
		void					Reset();

		//ĳ���˵��������ݴ��룬�õ��ض�����״̬��ͨ����ʼ�׶Σ�����min_timeʱ���ڣ�����������ƣ�֮�����������������״̬
		void					OnAction(const std::vector<sAction> &a, std::vector<sGesture> &g);//������յ��ֶ�������ô˽ӿں���

		//�����ȡ����ֵ���������ý���Ĭ��ֵ��ʵ�ʲ���һ��
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


	//����ʶ����
	__interface __declspec(uuid("{2A78DCDA-4D63-4EEA-B614-56E739F7C167}")) IKinectActionEvent :public IDLPUnknown{
		void OnAction(const std::vector<sAction1>&  sAction);
//		void  GetHands(std::vector<sAction1> & sHandActions);
	};
	typedef NSINSIVE_PTR::intrusive_ptr<IKinectActionEvent> PIKinectActionEvent;


	//query֧��IKinectDataEvent
	__interface __declspec(uuid("{EED3FE89-2B27-48E9-B0F0-8F31963ADA18}")) IKinectAction :public IDLPUnknown
	{

		//��������
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

	//IEvent:���ں�Gesture�����Լ���KinectActionEvent��������//
	typedef boost::function<void(std::vector<sGesture>)> mycallbackfunc;
	__interface __declspec(uuid("{F053564F-ABCD-4208-91F6-2FABD73FD840}")) IEvent : public IDLPUnknown
	{

		BOOL Init(PIDLPUnknown pKinectAction);   //��ʼ��m_PIKinectAction//
		BOOL SetEnableGesture(GestureEng::eGestureType gtype, bool bEnable); //�������������//
//		void GetGesture();                               //��ȡ����//

		void Registry(mycallbackfunc callbackfunc);

//		void Test(GestureEng::sGesture &s);

	};
	typedef NSINSIVE_PTR::intrusive_ptr<IEvent>  PIEvent;


	__interface __declspec(uuid("{C18BE961-7534-4003-A0FC-73ACD80A903B}")) ISynGesture : public IDLPUnknown
	{
		BOOL InitDevice(); //�����豸�������߳�//
		BOOL InitGesReg(const std::vector<nvmath::Vec3f> & vMarkScreemPoints, const std::vector<nvmath::Vec3f> & vMarkKinectPoints);
		BOOL Calib(const std::vector<nvmath::Vec3f> & vMarkScreemPoints, const std::vector<nvmath::Vec2f> &CalibPixelPoints);
		BOOL SetEnableGesture(eGestureType gtype, bool bEnable);
		void Registry(mycallbackfunc callbackfunc);
	};
	typedef NSINSIVE_PTR::intrusive_ptr<ISynGesture> PISynGesture;

	//-----------------------------------------------------------
	//3άץȡ
	//query֧��IKinectDataEvent
	__interface IKinectGrap :public IDLPUnknown{
		//��������
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
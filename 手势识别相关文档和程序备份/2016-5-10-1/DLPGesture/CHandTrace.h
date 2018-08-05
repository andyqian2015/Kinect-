#pragma once
#ifndef CHANDTRACE_H
#define CHANDTRACE_H

#include "fstream"

#include <time.h>

#include "dlpgesture\IDLPGesture.h"
#include "time.h"
#include "opencv/cv.h"
#include "opencv2/highgui/highgui.hpp"
#include "opencv/highgui.h"

#include "OpenExr/include/ImathVec.h"
#include "OpenExr/include/ImathLine.h"
#include "OpenExr/include/ImathLineAlgo.h"
#include "OpenExr/include/ImathPlane.h"
#include "OpenExr/include/ImathQuat.h"
using namespace IMATH_NAMESPACE;

using namespace GestureEng;

using namespace cv;

#define MAX_HAND_NUM 5
#define foc 370 

 
class CHandTrace : public IHandTrace, public CUnknown
{

public:

	DECLARE_IUNKNOWN
	STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void **ppv);

	BOOL                                                InitData(const int &ImgWidth, const int &ImgHeight,const std::vector<nvmath::Vec3f> & vmarkscreenpos, const std::vector<nvmath::Vec3f> & vmarkKinectPos, eInstallMode installMode, float h, float h1, float Radius);
	CHandTrace();
	~CHandTrace();
	BOOL                                                GetHandGesture(const void* pBuffer, int Imagewidth, int Imageheight, std::vector<sAction1> &handAction,INT64 utimestamp);


private:

	//对手势的尖端的深度修正//	
	void                                                 ReviseSummitDepth(const std::vector<sAction1> &vCopyPreHands, cv::Point3f &imgpt);
	//获取当前帧下的手势//
	void                                                 GetCurrentHandsCounter(const std::vector<sAction1> &vCopyPreHands);
	//利用当前帧的历史手势对当前手势进行更新
	void                                                 adjustPreGesture(const std::vector<sAction1> &vCopyPreHands);
	void                                                 updateCurrentGesture(const std::vector<sAction1> &vCopyPreHands);

	bool                                                 GetSummitPoint(float a,float b);  //正向安装为0，反向安装位1//
	
	bool                                                 SetKinectCoordinateSystem();  //利用标定板上的点的Kinect坐标，得到标定板的坐标系//

	void                                                 Mult(const V3f &vec, V3f &v);  //矩阵和向量相乘//

	void                                                 GetInentZone();  //获取标定板的识别区域//

	void                                                 GetReviseHandPos();

	void                                                 SeekHands(cv::Mat& cv16img,std::vector<sAction1> &hands);
	void                                                 getRatio(const V3f &a, const V3f &b, const V3f & c, V3f &point);
	void                                                 convertTo(const V3f& pos, V3f & Point);

	void                                                 getKinectPos(const V3f & pos, V3f & Point);
	void                                                 getPixelPos(const V3f & pos, V3f &pixelPoint);  //利用Kinect坐标获取像素点坐标//

	bool                                                 Precalc16_fast(cv::Mat& cv16img);

	bool                                                 IsBetweenTwoPlane(const V3f  & v1, const V3f  &v2, const V3f & v3);
	void                                                 GetPlane(V3f  & v1, V3f & v2, V3f & v3, V3f & v4, Plane3f &screenPlane1);
	bool                                                 IsInZone(const V3f  & pKinectPos);
	void                                                 getProjectPos(const V3f & sKinectPos, V3f & vProjectPoint);
	void                                                 getProjectPos(const V3f & sKinectPos, const Plane3f & plane, V3f &vProjectPoint);
	bool                                                  GetPositionLocation(const V3f & v1, const V3f & v2, const V3f & v3, const V3f & v4, const V3f & v5);

	bool                                                  GetHandPos(const Mat & cv16Img, const std::vector<Point>& contour, cv::Point3f  & hand);

	void										          CalcAve(const cv::Mat& cv16img, std::vector<Point>& contour, int  & maxdepth, int & mindepth, float  & avg);
	float												  CalcAveDepth(const cv::Mat& cv16img, std::vector<Point>& contour);

	void                                                 depthRevise(cv::Mat & cv16Img, std::vector<Point> & srcContours, std::vector<Point> & contours);    //对边缘的深度数据修正//

	bool                                                 isBorderOrOut(const cv::Mat & cv16Img, const std::vector<Point> & contours, const cv::Point & p,const float                                                                  &fBoundDepth );  //判断点是否在边界上//

	bool                                                 avgDepth(const cv::Point2f & p, const std::vector<Point> & srcContours, cv::Mat & cv16Img);  //给定某个像素点，取该像素点以及其八个方向，剔除掉边界点和外点，求该像素点的平均深度//

	//test for use//
	void                                                GetValidDepth(const cv::Point2f & p, const std::vector<Point> & srcContours, cv::Mat & cv16Img,                                                                     std::vector<cv::Point3f> &vOridinal, int &Count,float &depth);


	//test for use//
	BOOL                                                 GetAvgDepthAll(cv::Point2f & p, const std::vector<Point> & srcContours, cv::Mat & cv16Img,                                                                            std::vector<cv::Point2f> &pNeighbour);
	//test for use//

	void                                                 getMaxDepth(const std::vector<float>  & vOridinalDepth, cv::Point2f & Point2f);  //给定某个像素点，取该像素点以及其八个方向,获取其中有效点最大深度对应的序号和深度值//
	cv::Point2f                                          getMinDepth(const std::vector<float>  & vOridinalDepth);   //给定某个像素点，取该像素点以及其八个方向,获取其中有效点最小深度对应的序号和深度值//

	bool                                                 elimateBadDepth(const cv::Point2f  & pMaxDepth, const cv::Point2f  & pMinDepth, const float  & avgDepth, std::vector<float>  & vOridinalDepth); //排除掉坏点，即远离平均值的点//

	BOOL                                                 getValidInnerPoint(const cv::Mat & cv16Img, const std::vector<Point> & srcContours, cv::Point2f & pCenter); //考虑到边界点的深度值不稳定，将边界点的邻域中的内部点替换掉边界点//

	//test//
	BOOL                                                 getValidInnerPointAll(const cv::Mat & cv16Img, const std::vector<Point> & srcContours, cv::Point2f & p);
	void                                                 getNeighbour(cv::Point2f & p, std::vector<cv::Point2f>& vNeighbour);
	//test//

	//test for use//
	void                                                 GetPixelPoints(std::vector<V4f> &vpixelPoints);
	void                                                 GetRefPixelPointsAll();

	float                                                getAvgPixely(std::vector<cv::Point> & contours);

	void                                                 on_trackbar(int pos, std::vector<vector<Point>> & contours);

	void                                                  ConvertImagePointToHand(const V3f& p, sAction1& act, const std::vector<sAction1> preHands, const int MarchIndex); //用于获取手势的姿态//


	//用于计算点到平面的距离//
	void                                                 CalDistFromPointToPlane(const V3f &p, const Plane3f & plane, float dist);

	//讨论主持人在屏幕一侧的情形//
	bool                                                  getSummitPoint(const float  & threshold, const std::vector<cv::Point>  & srcContours, cv::Point  & fSummitPoint, const cv::Mat & cv16img);//若主持人在一侧，通过寻找极端的pixelx,pixely坐标得到手势尖端点的坐标//

	 bool                                                 IsInOrOutOfZone(const V3f  & KinectPos, const float & fSpeedLength, V3f vfPospre, V3f vfPosAfter);//判断点是否在边界上//

	 //利用K-Means算法剔除掉选出的5个点中的4个得到我们所需要的指尖点//
	 void                                                 getSummitPoint(std::vector<cv::Point> & contours, cv::Point & fsummit);
	 void                                                 UpdateGroup(const std::vector<cv::Point> & vMarkPoint, const std::vector<cv::Point>  & srcContours, std::vector<std::vector<cv::Point>> &vGroups);
	 float                                                getMin(std::vector<float> vDist);
	 float                                                getAvgDepth(const cv::Mat & cv16Img, std::vector<cv::Point> & contours);
	 //利用K-Means算法剔除掉选出的5个点中的4个得到我们所需要的指尖点//

	 //用于获取到当前帧数据与前一帧手势数据中的对应pos//
	 bool                                                  getMarchLastHandPos(const std::vector<sAction1>  & vLastHand, const cv::Point3f  & kinectPos, const float &                                                                                threshold, int  & index);
	 UINT                                                  getMaxDepth();
	 UINT                                                  getMinDepth();

	 //test//
	 bool                                                 GetSummitPoint(const std::vector<sAction1> &vCopyPreHands);

	 //将所有获取手势尖端的方法放置在一个函数里
	 void                                                 GetSummitPointAll(const std::vector<sAction1> &vCopyPreHands); 

	
	 void                                                GetCounterExtremePoint(const std::vector<cv::Point> &counters, const cv::Point2f &cen, const float &radius);//根据                                                    现有轮廓，获取轮廓的极端点//

	 void                                                NonInitFrame(const std::vector<sAction1> &vCopyPreHands); //当非初始帧时，获取vpos//
	 void                                                InitFrame(const std::vector<sAction1> &vCopyPreHands); //当为初始帧时，获取vPos//

	
	 void                                                MatchToLastFrame_1(const std::vector<sAction1> &vCopyPreHands, std::vector<cv::Point3f> &vfPos);  //对当前帧比前一帧                                                                                                                                                   多的情形下的匹配// 
	 void                                                MatchToLastFrame_2(const std::vector<sAction1> &vCopyPreHands, std::vector<cv::Point3f> &vfPos); //当前帧比前一帧少                                                                                                                                                    下的匹配//
	
	 void                                                GetCompensatePos();

	 void                                               UpdateHand(const std::vector<sAction1> &vCopyPreHands);


	 void                                               ReviseHandID(const std::vector<sAction1> &vCopyPreHands);

	 float                                                m_fHandThreshold;

	 //test//
	 void                                                ReviseHands(const std::vector<sAction1> &vCopyPreHands);
	 void                                                deleteHands(const std::vector<sAction1> &vCopyPreHands);


	 //test for use//
	 void                                                pixelRefMatch();

	 void                                                fsummitPointMarch(const float & fcurdepth, int &index, int &MinValue, float &                                                         dDepth);
	 void                                                GetPixelRefAvg(const int &index, float & favg);

	 BOOL                                                fsummitPointRevise( V3f &pixelPoint);
	 void                                                DeleteUpHands(const std::vector<sAction1> &vCopyPreHands);

	 //test for use//
	 void                                                GetMarch(const std::vector<sAction1> &vCopyPreHands, std::vector<int> &vIndex);
	 //test for use//

	 Matrix44<float>                                      m_invTransFromMat;              //获取Kinect坐标在标定平面的坐标//
	 V3f                                                  m_vProToCalibPlanePos;         //将Kinect坐标点投影到标定平面坐标系下的点的坐标,只需要X,Y坐标//

	 float                                                m_hDistScreen; //确定手势为Enter手势//
	std::vector<V3f>                                      m_vpixelKinectPos;
	int                                                   m_KinectInstallPattern;

	//在判断边缘轮廓时，可以先找最最大矩形范围，在最大矩形范围里找平行四边形，得到最合适的识别区域//
	int                                                   m_cDepthWidth = 0;
	int                                                   m_cDepthHeight = 0;

	cv::Mat                                               m_cv16Img;

	std::vector<sAction1>                                 m_hands;

	std::vector<V3f>                                      m_vKinectPos;   //标记点的kinect坐标//
	std::vector<V3f>                                      m_sCreenPos;    //标记点的屏幕坐标//

	//利用像素点确定识别区域，使得识别的区域较小，可以改用Kinect坐标系下的x,y,z坐标进行确定//

	float                                                m_iMaxLimitx;
	float                                                m_iMaxLimity;
	float                                                m_iMaxLimitz;
	float                                                m_iMinLimitx;
	float                                                m_iMinLimity;
	float                                                m_iMinLimitz;

	//标定点的最小最大像素点//
	float                                                m_iMaxLimitx_1;
	float                                                m_iMaxLimity_1;
	float                                                m_iMinLimitx_1;
	float                                                m_iMinLimity_1;
	float                                                m_iMaxLimitz_1;
	float                                                m_iMinLimitz_1;

	//识别手势的尖端//
	float                                                m_minPixely;

	//利用Kinect的标定平面确定手势的识别区域//
	float                                                 m_fMaxIdentX;  //标定板识别区域的阈值//
	float                                                 m_fMinIdentX;
	float                                                 m_fMaxIdentY;
	float                                                 m_fMinIdentY;

	float                                                 m_fOutThreshold;
	int                                                   m_markpixelx;
	int                                                   m_markpixely;

	int                                                   m_iPointthreshold;   //对于有可能出现的，找到的较大值点总在已经找到的边界轮廓附近的情况，使得后面的非需要的边界轮廓点出现在视野中，采取的方法为，设定点的阈值//
	int                                                   m_depthrevisethreshold;  //当该点的深度和平均值的深度差在阈值以内，我们认为该点的深度是可靠的，可以不需要对深度进行修正//
	float                                                 m_Radius;               //手势保持静止的半径阈值//
	float                                                 m_h;                    //手势活动的深度//

	Plane3f                                               m_pScreenPlane;          //屏幕平面//

	Plane3f                                               m_pdisScreenPlane;          //离屏平面//

	std::vector<V3f>                                      m_vdisMarkPoint;           //离屏平面的四个限制点//

	//for revise depth//
	cv::Point                                             m_fSummitPoint;

	int                                                   m_identPixelx3;
	int                                                   m_identPixely3;
	//用于判定失去的手势的原因是离开识别区域或者是检测不到引起的HandAction的丢失，flag =true：表示离开识别区域;flag = false表示，由于点未检测到引起的//
	bool                                                  m_bIsInOrOutFlag;

	float                                                 m_fActorOnOneSidethreshold; //主持人是否在一侧的阈值//

	//用于判断匹配的手势是否是同一个手势的阈值//
	std::vector<float>                                    m_vHandPosThreshold;
	float                                                 m_fHandPosThreshold;  //用于辅助测试,判断是否是同一个手势://
	//用于判断匹配的手势是否是同一个手势的阈值//

	std::vector <V3f>                                     m_vfKinectPos;    //为当前手势的KinectPos//

	//用于测试修正的深度是否适合//

	std::vector<std::vector<cv::Point3f>>                 m_vPos;                   //记录前两帧对应的手势的pos//

	std::vector<vector<Point>>                            m_contours;
	std::vector<cv::Point>                                m_MyContours;

	UINT                                                  m_iMaxDepth;    //限制区域的最大深度//
	UINT                                                  m_iMinDepth;    //限制区域的最小深度//

 	cv::Mat                                              m_cv8Img;
	std::vector<cv::Point>                               m_MyContours1;

	//test for use//
	std::vector<std::vector<V4f>>                       m_vPixelPointRef;
	std::vector<BOOL>                                    m_vflag;
	

	//删除错误手势//
	int                                                 m_testUP = 0;
	int                                                 m_testDown = 0;
	INT64                                               m_timebegin = 0;
	INT64                                               m_timeend = 0;

	//test//
	V3f                                                  m_PixelPos;
	std::vector<cv::Point>                               m_copyMyContours;
	int                                                  m_testArea = 0;
	cv::Point                                           m_testfSummitPoint;
	std::vector<cv::Point>                              m_copyMyContours1;

	std::vector<std::vector<cv::Point> >                m_vcopyMyCounters1;


	IplImage *                                          m_g_pGrayImage = NULL;
	IplImage *                                          m_g_pBinaryImage = NULL;

	//test for use//
	std::vector<int>                                    m_vIndex; //当前帧和前一帧的数据的匹配指数//
};

#endif
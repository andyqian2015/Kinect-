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

	//�����Ƶļ�˵��������//	
	void                                                 ReviseSummitDepth(const std::vector<sAction1> &vCopyPreHands, cv::Point3f &imgpt);
	//��ȡ��ǰ֡�µ�����//
	void                                                 GetCurrentHandsCounter(const std::vector<sAction1> &vCopyPreHands);
	//���õ�ǰ֡����ʷ���ƶԵ�ǰ���ƽ��и���
	void                                                 adjustPreGesture(const std::vector<sAction1> &vCopyPreHands);
	void                                                 updateCurrentGesture(const std::vector<sAction1> &vCopyPreHands);

	bool                                                 GetSummitPoint(float a,float b);  //����װΪ0������װλ1//
	
	bool                                                 SetKinectCoordinateSystem();  //���ñ궨���ϵĵ��Kinect���꣬�õ��궨�������ϵ//

	void                                                 Mult(const V3f &vec, V3f &v);  //������������//

	void                                                 GetInentZone();  //��ȡ�궨���ʶ������//

	void                                                 GetReviseHandPos();

	void                                                 SeekHands(cv::Mat& cv16img,std::vector<sAction1> &hands);
	void                                                 getRatio(const V3f &a, const V3f &b, const V3f & c, V3f &point);
	void                                                 convertTo(const V3f& pos, V3f & Point);

	void                                                 getKinectPos(const V3f & pos, V3f & Point);
	void                                                 getPixelPos(const V3f & pos, V3f &pixelPoint);  //����Kinect�����ȡ���ص�����//

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

	void                                                 depthRevise(cv::Mat & cv16Img, std::vector<Point> & srcContours, std::vector<Point> & contours);    //�Ա�Ե�������������//

	bool                                                 isBorderOrOut(const cv::Mat & cv16Img, const std::vector<Point> & contours, const cv::Point & p,const float                                                                  &fBoundDepth );  //�жϵ��Ƿ��ڱ߽���//

	bool                                                 avgDepth(const cv::Point2f & p, const std::vector<Point> & srcContours, cv::Mat & cv16Img);  //����ĳ�����ص㣬ȡ�����ص��Լ���˸������޳����߽�����㣬������ص��ƽ�����//

	//test for use//
	void                                                GetValidDepth(const cv::Point2f & p, const std::vector<Point> & srcContours, cv::Mat & cv16Img,                                                                     std::vector<cv::Point3f> &vOridinal, int &Count,float &depth);


	//test for use//
	BOOL                                                 GetAvgDepthAll(cv::Point2f & p, const std::vector<Point> & srcContours, cv::Mat & cv16Img,                                                                            std::vector<cv::Point2f> &pNeighbour);
	//test for use//

	void                                                 getMaxDepth(const std::vector<float>  & vOridinalDepth, cv::Point2f & Point2f);  //����ĳ�����ص㣬ȡ�����ص��Լ���˸�����,��ȡ������Ч�������ȶ�Ӧ����ź����ֵ//
	cv::Point2f                                          getMinDepth(const std::vector<float>  & vOridinalDepth);   //����ĳ�����ص㣬ȡ�����ص��Լ���˸�����,��ȡ������Ч����С��ȶ�Ӧ����ź����ֵ//

	bool                                                 elimateBadDepth(const cv::Point2f  & pMaxDepth, const cv::Point2f  & pMinDepth, const float  & avgDepth, std::vector<float>  & vOridinalDepth); //�ų������㣬��Զ��ƽ��ֵ�ĵ�//

	BOOL                                                 getValidInnerPoint(const cv::Mat & cv16Img, const std::vector<Point> & srcContours, cv::Point2f & pCenter); //���ǵ��߽������ֵ���ȶ������߽��������е��ڲ����滻���߽��//

	//test//
	BOOL                                                 getValidInnerPointAll(const cv::Mat & cv16Img, const std::vector<Point> & srcContours, cv::Point2f & p);
	void                                                 getNeighbour(cv::Point2f & p, std::vector<cv::Point2f>& vNeighbour);
	//test//

	//test for use//
	void                                                 GetPixelPoints(std::vector<V4f> &vpixelPoints);
	void                                                 GetRefPixelPointsAll();

	float                                                getAvgPixely(std::vector<cv::Point> & contours);

	void                                                 on_trackbar(int pos, std::vector<vector<Point>> & contours);

	void                                                  ConvertImagePointToHand(const V3f& p, sAction1& act, const std::vector<sAction1> preHands, const int MarchIndex); //���ڻ�ȡ���Ƶ���̬//


	//���ڼ���㵽ƽ��ľ���//
	void                                                 CalDistFromPointToPlane(const V3f &p, const Plane3f & plane, float dist);

	//��������������Ļһ�������//
	bool                                                  getSummitPoint(const float  & threshold, const std::vector<cv::Point>  & srcContours, cv::Point  & fSummitPoint, const cv::Mat & cv16img);//����������һ�࣬ͨ��Ѱ�Ҽ��˵�pixelx,pixely����õ����Ƽ�˵������//

	 bool                                                 IsInOrOutOfZone(const V3f  & KinectPos, const float & fSpeedLength, V3f vfPospre, V3f vfPosAfter);//�жϵ��Ƿ��ڱ߽���//

	 //����K-Means�㷨�޳���ѡ����5�����е�4���õ���������Ҫ��ָ���//
	 void                                                 getSummitPoint(std::vector<cv::Point> & contours, cv::Point & fsummit);
	 void                                                 UpdateGroup(const std::vector<cv::Point> & vMarkPoint, const std::vector<cv::Point>  & srcContours, std::vector<std::vector<cv::Point>> &vGroups);
	 float                                                getMin(std::vector<float> vDist);
	 float                                                getAvgDepth(const cv::Mat & cv16Img, std::vector<cv::Point> & contours);
	 //����K-Means�㷨�޳���ѡ����5�����е�4���õ���������Ҫ��ָ���//

	 //���ڻ�ȡ����ǰ֡������ǰһ֡���������еĶ�Ӧpos//
	 bool                                                  getMarchLastHandPos(const std::vector<sAction1>  & vLastHand, const cv::Point3f  & kinectPos, const float &                                                                                threshold, int  & index);
	 UINT                                                  getMaxDepth();
	 UINT                                                  getMinDepth();

	 //test//
	 bool                                                 GetSummitPoint(const std::vector<sAction1> &vCopyPreHands);

	 //�����л�ȡ���Ƽ�˵ķ���������һ��������
	 void                                                 GetSummitPointAll(const std::vector<sAction1> &vCopyPreHands); 

	
	 void                                                GetCounterExtremePoint(const std::vector<cv::Point> &counters, const cv::Point2f &cen, const float &radius);//����                                                    ������������ȡ�����ļ��˵�//

	 void                                                NonInitFrame(const std::vector<sAction1> &vCopyPreHands); //���ǳ�ʼ֡ʱ����ȡvpos//
	 void                                                InitFrame(const std::vector<sAction1> &vCopyPreHands); //��Ϊ��ʼ֡ʱ����ȡvPos//

	
	 void                                                MatchToLastFrame_1(const std::vector<sAction1> &vCopyPreHands, std::vector<cv::Point3f> &vfPos);  //�Ե�ǰ֡��ǰһ֡                                                                                                                                                   ��������µ�ƥ��// 
	 void                                                MatchToLastFrame_2(const std::vector<sAction1> &vCopyPreHands, std::vector<cv::Point3f> &vfPos); //��ǰ֡��ǰһ֡��                                                                                                                                                    �µ�ƥ��//
	
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

	 Matrix44<float>                                      m_invTransFromMat;              //��ȡKinect�����ڱ궨ƽ�������//
	 V3f                                                  m_vProToCalibPlanePos;         //��Kinect�����ͶӰ���궨ƽ������ϵ�µĵ������,ֻ��ҪX,Y����//

	 float                                                m_hDistScreen; //ȷ������ΪEnter����//
	std::vector<V3f>                                      m_vpixelKinectPos;
	int                                                   m_KinectInstallPattern;

	//���жϱ�Ե����ʱ�����������������η�Χ���������η�Χ����ƽ���ı��Σ��õ�����ʵ�ʶ������//
	int                                                   m_cDepthWidth = 0;
	int                                                   m_cDepthHeight = 0;

	cv::Mat                                               m_cv16Img;

	std::vector<sAction1>                                 m_hands;

	std::vector<V3f>                                      m_vKinectPos;   //��ǵ��kinect����//
	std::vector<V3f>                                      m_sCreenPos;    //��ǵ����Ļ����//

	//�������ص�ȷ��ʶ������ʹ��ʶ��������С�����Ը���Kinect����ϵ�µ�x,y,z�������ȷ��//

	float                                                m_iMaxLimitx;
	float                                                m_iMaxLimity;
	float                                                m_iMaxLimitz;
	float                                                m_iMinLimitx;
	float                                                m_iMinLimity;
	float                                                m_iMinLimitz;

	//�궨�����С������ص�//
	float                                                m_iMaxLimitx_1;
	float                                                m_iMaxLimity_1;
	float                                                m_iMinLimitx_1;
	float                                                m_iMinLimity_1;
	float                                                m_iMaxLimitz_1;
	float                                                m_iMinLimitz_1;

	//ʶ�����Ƶļ��//
	float                                                m_minPixely;

	//����Kinect�ı궨ƽ��ȷ�����Ƶ�ʶ������//
	float                                                 m_fMaxIdentX;  //�궨��ʶ���������ֵ//
	float                                                 m_fMinIdentX;
	float                                                 m_fMaxIdentY;
	float                                                 m_fMinIdentY;

	float                                                 m_fOutThreshold;
	int                                                   m_markpixelx;
	int                                                   m_markpixely;

	int                                                   m_iPointthreshold;   //�����п��ܳ��ֵģ��ҵ��Ľϴ�ֵ�������Ѿ��ҵ��ı߽����������������ʹ�ú���ķ���Ҫ�ı߽��������������Ұ�У���ȡ�ķ���Ϊ���趨�����ֵ//
	int                                                   m_depthrevisethreshold;  //���õ����Ⱥ�ƽ��ֵ����Ȳ�����ֵ���ڣ�������Ϊ�õ������ǿɿ��ģ����Բ���Ҫ����Ƚ�������//
	float                                                 m_Radius;               //���Ʊ��־�ֹ�İ뾶��ֵ//
	float                                                 m_h;                    //���ƻ�����//

	Plane3f                                               m_pScreenPlane;          //��Ļƽ��//

	Plane3f                                               m_pdisScreenPlane;          //����ƽ��//

	std::vector<V3f>                                      m_vdisMarkPoint;           //����ƽ����ĸ����Ƶ�//

	//for revise depth//
	cv::Point                                             m_fSummitPoint;

	int                                                   m_identPixelx3;
	int                                                   m_identPixely3;
	//�����ж�ʧȥ�����Ƶ�ԭ�����뿪ʶ����������Ǽ�ⲻ�������HandAction�Ķ�ʧ��flag =true����ʾ�뿪ʶ������;flag = false��ʾ�����ڵ�δ��⵽�����//
	bool                                                  m_bIsInOrOutFlag;

	float                                                 m_fActorOnOneSidethreshold; //�������Ƿ���һ�����ֵ//

	//�����ж�ƥ��������Ƿ���ͬһ�����Ƶ���ֵ//
	std::vector<float>                                    m_vHandPosThreshold;
	float                                                 m_fHandPosThreshold;  //���ڸ�������,�ж��Ƿ���ͬһ������://
	//�����ж�ƥ��������Ƿ���ͬһ�����Ƶ���ֵ//

	std::vector <V3f>                                     m_vfKinectPos;    //Ϊ��ǰ���Ƶ�KinectPos//

	//���ڲ�������������Ƿ��ʺ�//

	std::vector<std::vector<cv::Point3f>>                 m_vPos;                   //��¼ǰ��֡��Ӧ�����Ƶ�pos//

	std::vector<vector<Point>>                            m_contours;
	std::vector<cv::Point>                                m_MyContours;

	UINT                                                  m_iMaxDepth;    //���������������//
	UINT                                                  m_iMinDepth;    //�����������С���//

 	cv::Mat                                              m_cv8Img;
	std::vector<cv::Point>                               m_MyContours1;

	//test for use//
	std::vector<std::vector<V4f>>                       m_vPixelPointRef;
	std::vector<BOOL>                                    m_vflag;
	

	//ɾ����������//
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
	std::vector<int>                                    m_vIndex; //��ǰ֡��ǰһ֡�����ݵ�ƥ��ָ��//
};

#endif
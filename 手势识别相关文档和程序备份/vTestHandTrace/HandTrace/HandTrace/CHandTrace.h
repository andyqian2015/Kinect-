#include "IHandTrace.h"
#include "atltrace.h"
#include "time.h"
using namespace IMATH_NAMESPACE;

#define MAX_HAND_NUM 5
#define foc 370 


class CHandTrace : public HandTrace::IHandTrace {

	static const int        cDepthWidth = 512;
	static const int        cDepthHeight = 424;
public:
	void InitData(const std::vector<V3f> & vmarkscreenpos, const std::vector<V3f> & vmarkKinectPos, int KinectInstallPattern, float h, float h1, float Radius);
	std::vector<sAction> GetHandGesture(cv::Mat &cv16Img);
	//test//
	void SetVidient();
	std::vector<int>   TestGetUseTime();

	float TestGetSummity();

	//test//
	std::vector<cv::Point2f> GetCounterSummitPoint();  //��ȡ������˵㣬��ӡ����//
	std::vector<cv::Point3f> CHandTrace::GetCounterSummitPoint3();

	//test:���Լ��������ٸ���//
	int GetHandNum();


private:

	//���жϱ�Ե����ʱ�����������������η�Χ���������η�Χ����ƽ���ı��Σ��õ�����ʵ�ʶ������//

	int                                                   m_HandNum;  //Ϊ���Լ������ٸ��ַ���//

	std::vector<sAction>                                  m_hands;


	std::vector<V3f>                                      vKinectPos;   //��ǵ��kinect����//
	std::vector<V3f>                                      sCreenPos;    //��ǵ����Ļ����//


	//�������ص�ȷ��ʶ������ʹ��ʶ��������С�����Ը���Kinect����ϵ�µ�x,y,z�������ȷ��//

	std::vector<cv::Point>                             vSummitPoint; //�����ã�����ȷ���ж��ٸ�����//

	float                                                iMaxLimitx;
	float                                                iMaxLimity;
	float                                                iMaxLimitz;
	float                                                iMinLimitx;
	float                                                iMinLimity;
	float                                                iMinLimitz;


	//�궨�����С������ص�//
	float                                                iMaxLimitx_1;
	float                                                iMaxLimity_1;
	float                                                iMinLimitx_1;
	float                                                iMinLimity_1; 
	float                                                iMaxLimitz_1;
	float                                                iMinLimitz_1;


	float                                                iMaxLimitx_2;
	float                                                iMinLimitx_2;
	float                                                iMaxLimity_2;
	float                                                iMinLimity_2;

	std::vector<V3f>                                      vpixelKinectPos;

	int                                                   KinectInstallPattern;
	bool                                                  GetSummitPoint(float a,float b);  //����װΪ0������װλ1//
	int                                                   GetMinIndex(std::vector<cv::Point> &vGroups); //��ȡ���������ص�ֵ��С�ĵ�//
	int                                                   GetMaxIndex(std::vector<cv::Point> &vGroups); //��ȡ���������ص�ֵ�ϴ�ĵ�//


	bool                                                  SetKinectCoordinateSystem();  //���ñ궨���ϵĵ��Kinect���꣬�õ��궨�������ϵ//
	Matrix44<float>                                       invTransFromMat;              //��ȡKinect�����ڱ궨ƽ�������//
	V3f                                                   vProToCalibPlanePos;         //��Kinect�����ͶӰ���궨ƽ������ϵ�µĵ������,ֻ��ҪX,Y����//
	V3f                                                   Mult(V3f vec);   //������������//

	//����Kinect�ı궨ƽ��ȷ�����Ƶ�ʶ������//
	float                                                 m_fMaxIdentX;  //�궨��ʶ���������ֵ//
	float                                                 m_fMinIdentX;
	float                                                 m_fMaxIdentY;
	float                                                 m_fMinIdentY;
	void                                                  GetInentZone();  //��ȡ�궨���ʶ������//

	float                                                 m_fHandThreshold;
	void                                                  GetReviseHandPos();

	int                                                   SeekHands(cv::Mat& cv16img);
	V3f                                                   getRatio(const V3f &a, const V3f &b, const V3f & c);
	V3f                                                   convertTo(const V3f& pos);
	V3f                                                   getKinectPos(const V3f & pos);
	V3f                                                   getPixelPos(const V3f & pos); //����Kinect�����ȡ���ص�����//

	bool                                                  Precalc16_fast(cv::Mat& cv16img);

	bool                                                  IsBetweenTwoPlane(const V3f  & v1, const V3f  &v2, const V3f & v3);
	Plane3f                                               GetPlane(V3f  & v1, V3f & v2, V3f & v3, V3f & v4);
	bool                                                  IsInZone(const V3f  & pKinectPos);
	V3f                                                   getProjectPos(const V3f & sKinectPos);
	V3f                                                   getProjectPos(const V3f & sKinectPos, const Plane3f & plane);
	bool                                                  GetPositionLocation(const V3f & v1, const V3f & v2, const V3f & v3, const V3f & v4, const V3f & v5);

	bool                                                  GetHandPos(const Mat & cv16Img, const std::vector<Point>& contour, cv::Point3f  & hand);

	void										          CalcAve(const cv::Mat& cv16img, std::vector<Point>& contour, int  & maxdepth, int & mindepth, float  & avg);
	float												  CalcAveDepth(const cv::Mat& cv16img, std::vector<Point>& contour);


	float                                                h1; //ȷ������ΪEnter����//


	void                                                 depthRevise(cv::Mat & cv16Img, std::vector<Point> & srcContours, std::vector<Point> & contours);    //�Ա�Ե�������������//

	bool                                                 isBorderOrOut(const cv::Mat & cv16Img, const std::vector<Point> & contours, const cv::Point & p);  //�жϵ��Ƿ��ڱ߽���//

	bool                                                 avgDepth(const cv::Point2f & p, const std::vector<Point> & srcContours, cv::Mat & cv16Img);  //����ĳ�����ص㣬ȡ�����ص��Լ���˸������޳����߽�����㣬������ص��ƽ�����//

	cv::Point2f                                          getMaxDepth(const std::vector<float>  & vOridinalDepth);   //����ĳ�����ص㣬ȡ�����ص��Լ���˸�����,��ȡ������Ч�������ȶ�Ӧ����ź����ֵ//
	cv::Point2f                                          getMinDepth(const std::vector<float>  & vOridinalDepth);   //����ĳ�����ص㣬ȡ�����ص��Լ���˸�����,��ȡ������Ч����С��ȶ�Ӧ����ź����ֵ//

	bool                                                 elimateBadDepth(const cv::Point2f  & pMaxDepth, const cv::Point2f  & pMinDepth, const float  & avgDepth, std::vector<float>  & vOridinalDepth); //�ų������㣬��Զ��ƽ��ֵ�ĵ�//

	void                                                 getValidInnerPoint(const cv::Mat & cv16Img, const std::vector<Point> & srcContours, cv::Point2f & pCenter); //���ǵ��߽������ֵ���ȶ������߽��������е��ڲ����滻���߽��//

	std::vector<cv::Point2f>                             getNeighbour(const cv::Point2f & p);       //��ȡ��ĳ����������еĵ�//
	float                                                getAvgPixely(std::vector<cv::Point> & contours);


	//for test//
	void                                                 on_trackbar(int pos, std::vector<vector<Point>> & contours);

	float                                                fOutThreshold;


	void                                                 setPixel(int markpixelx, int markpixely);

	void                                                 ConvertImagePointToHand(const V3f& img, sAction& act, const std::vector<sAction> preHands);  //���ڻ�ȡ���Ƶ���̬//


	//test://
	//���ڼ���㵽ƽ��ľ���//
	void                                                 CalDistFromPointToPlane(const V3f &p, const Plane3f & plane, float dist);
	//���ڼ���㵽ƽ��ľ���//
	//test://

	//test//
	//��������������Ļһ�������//
	bool                                                  getSummitPoint(const float  & threshold, const std::vector<cv::Point>  & srcContours, cv::Point  & fSummitPoint, const cv::Mat & cv16img);//����������һ�࣬ͨ��Ѱ�Ҽ��˵�pixelx,pixely����õ����Ƽ�˵������//
	//��������������Ļһ�������//
	//test//

	bool                                                  IsInOrOutOfZone(const V3f  & KinectPos, const float & fSpeedLength, V3f  vfPospre, V3f vfPosAfter);//�жϵ��Ƿ��ڱ߽���//

	int                                                   markpixelx;
	int                                                   markpixely;

	int                                                   iPointthreshold;   //�����п��ܳ��ֵģ��ҵ��Ľϴ�ֵ�������Ѿ��ҵ��ı߽����������������ʹ�ú���ķ���Ҫ�ı߽��������������Ұ�У���ȡ�ķ���Ϊ���趨�����ֵ//
	int                                                   depthrevisethreshold;  //���õ����Ⱥ�ƽ��ֵ����Ȳ�����ֵ���ڣ�������Ϊ�õ������ǿɿ��ģ����Բ���Ҫ����Ƚ�������//
	float                                                 Radius;               //���Ʊ��־�ֹ�İ뾶��ֵ//
	float                                                 h;                    //���ƻ�����//

	Plane3f                                               pScreenPlane;          //��Ļƽ��//

	Plane3f                                               pdisScreenPlane;          //����ƽ��//

	std::vector<V3f>                                      vdisMarkPoint;           //����ƽ����ĸ����Ƶ�//

	int                                                   identPixelx;
	int                                                   identPixely;


	//test//
	std::vector<int>                                      videntPixelx;
	std::vector<int>                                      videntPixely;


	//for revise depth//
	cv::Point                                             fSummitPoint;

	//for test//
	int                                                   identPixelx1;
	int                                                   identPixely1;


	int                                                   identPixelx2;
	int                                                   identPixely2;

	int                                                   identPixelx3;
	int                                                   identPixely3;
	//�����ж�ʧȥ�����Ƶ�ԭ�����뿪ʶ����������Ǽ�ⲻ�������HandAction�Ķ�ʧ��flag =true����ʾ�뿪ʶ������;flag = false��ʾ�����ڵ�δ��⵽�����//
	bool                                                  bIsInOrOutFlag;



	float                                                 fActorOnOneSidethreshold; //�������Ƿ���һ�����ֵ//

	cv::Point2f                                           m_CritMarkPoint;        //�����ж����Ƶ��˶�״̬������//

	//�����ж�ƥ��������Ƿ���ͬһ�����Ƶ���ֵ//
	std::vector<float>                                    vHandPosThreshold;
	float                                                 fHandPosThreshold;  //���ڸ�������,�ж��Ƿ���ͬһ������://
	//�����ж�ƥ��������Ƿ���ͬһ�����Ƶ���ֵ//

	std::vector <V3f>                                     vfKinectPos;    //Ϊ��ǰ���Ƶ�KinectPos//


	//���ڲ�������������Ƿ��ʺ�//

	std::vector<cv::Point>                                identPixel;

	std::vector<std::vector<cv::Point3f>>                 vPos;                   //��¼ǰ��֡��Ӧ�����Ƶ�pos//


	//���ڻ�ȡ����ǰ֡������ǰһ֡���������еĶ�Ӧpos//
	bool                                                  getMarchLastHandPos(const std::vector<sAction>  & vLastHand, const cv::Point3f  & kinectPos, const float & threshold, int  & index);



	//for test//

	UINT                                                  getMaxDepth();
	UINT                                                  getMinDepth();

	UINT                                                  iMaxDepth;    //���������������//
	UINT                                                  iMinDepth;    //�����������С���//

	int                                                   minPixelx;   //��Ļ��Ե���Ƶ������С����//     //UINT��int��������õ����Ҳ��UINT//
	int                                                   maxPixelx;   //��Ļ��Ե���Ƶ������С����//
	int                                                   minPixely;   //��Ļ��Ե���Ƶ������С����//
	int                                                   maxPixely;   //��Ļ��Ե���Ƶ������С����//

 	cv::Mat                                              cv8Img;

	//for test//
	cv::Mat                                              cv16Img;

	//for test//

	//test//
	int                                                 useTime1;
	int                                                 useTime2;
	float                                               m_fSummity;
	void                                                TestSetSummity(float fSummity);

	//test//



	//����K-Means�㷨�޳���ѡ����5�����е�4���õ���������Ҫ��ָ���//
	cv::Point2f                                          getSummitPoint(std::vector<cv::Point> & contours);
	std::vector<std::vector<cv::Point>>                  getGroup(const int & k, const std::vector<cv::Point>  & srcContours, const float  & threshold);
	void                                                 UpdateGroup(const std::vector<cv::Point> & vMarkPoint, const std::vector<cv::Point>  & srcContours, std::vector<std::vector<cv::Point>> &vGroups);
	float                                                getMin(std::vector<float> vDist);
	cv::Point                                            getOptimalPoint(const std::vector<cv::Point> & srcContours);
	float                                                getAvgDepth(const cv::Mat & cv16Img, std::vector<cv::Point> & contours);
	//����K-Means�㷨�޳���ѡ����5�����е�4���õ���������Ҫ��ָ���//
};
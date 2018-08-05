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
	std::vector<cv::Point2f> GetCounterSummitPoint();  //获取轮廓尖端点，打印出来//
	std::vector<cv::Point3f> CHandTrace::GetCounterSummitPoint3();

	//test:测试检测出来多少个手//
	int GetHandNum();


private:

	//在判断边缘轮廓时，可以先找最最大矩形范围，在最大矩形范围里找平行四边形，得到最合适的识别区域//

	int                                                   m_HandNum;  //为测试检测出多少个手服务//

	std::vector<sAction>                                  m_hands;


	std::vector<V3f>                                      vKinectPos;   //标记点的kinect坐标//
	std::vector<V3f>                                      sCreenPos;    //标记点的屏幕坐标//


	//利用像素点确定识别区域，使得识别的区域较小，可以改用Kinect坐标系下的x,y,z坐标进行确定//

	std::vector<cv::Point>                             vSummitPoint; //测试用，用于确定有多少个手势//

	float                                                iMaxLimitx;
	float                                                iMaxLimity;
	float                                                iMaxLimitz;
	float                                                iMinLimitx;
	float                                                iMinLimity;
	float                                                iMinLimitz;


	//标定点的最小最大像素点//
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
	bool                                                  GetSummitPoint(float a,float b);  //正向安装为0，反向安装位1//
	int                                                   GetMinIndex(std::vector<cv::Point> &vGroups); //获取轮廓中像素点值较小的点//
	int                                                   GetMaxIndex(std::vector<cv::Point> &vGroups); //获取轮廓中像素点值较大的点//


	bool                                                  SetKinectCoordinateSystem();  //利用标定板上的点的Kinect坐标，得到标定板的坐标系//
	Matrix44<float>                                       invTransFromMat;              //获取Kinect坐标在标定平面的坐标//
	V3f                                                   vProToCalibPlanePos;         //将Kinect坐标点投影到标定平面坐标系下的点的坐标,只需要X,Y坐标//
	V3f                                                   Mult(V3f vec);   //矩阵和向量相乘//

	//利用Kinect的标定平面确定手势的识别区域//
	float                                                 m_fMaxIdentX;  //标定板识别区域的阈值//
	float                                                 m_fMinIdentX;
	float                                                 m_fMaxIdentY;
	float                                                 m_fMinIdentY;
	void                                                  GetInentZone();  //获取标定板的识别区域//

	float                                                 m_fHandThreshold;
	void                                                  GetReviseHandPos();

	int                                                   SeekHands(cv::Mat& cv16img);
	V3f                                                   getRatio(const V3f &a, const V3f &b, const V3f & c);
	V3f                                                   convertTo(const V3f& pos);
	V3f                                                   getKinectPos(const V3f & pos);
	V3f                                                   getPixelPos(const V3f & pos); //利用Kinect坐标获取像素点坐标//

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


	float                                                h1; //确定手势为Enter手势//


	void                                                 depthRevise(cv::Mat & cv16Img, std::vector<Point> & srcContours, std::vector<Point> & contours);    //对边缘的深度数据修正//

	bool                                                 isBorderOrOut(const cv::Mat & cv16Img, const std::vector<Point> & contours, const cv::Point & p);  //判断点是否在边界上//

	bool                                                 avgDepth(const cv::Point2f & p, const std::vector<Point> & srcContours, cv::Mat & cv16Img);  //给定某个像素点，取该像素点以及其八个方向，剔除掉边界点和外点，求该像素点的平均深度//

	cv::Point2f                                          getMaxDepth(const std::vector<float>  & vOridinalDepth);   //给定某个像素点，取该像素点以及其八个方向,获取其中有效点最大深度对应的序号和深度值//
	cv::Point2f                                          getMinDepth(const std::vector<float>  & vOridinalDepth);   //给定某个像素点，取该像素点以及其八个方向,获取其中有效点最小深度对应的序号和深度值//

	bool                                                 elimateBadDepth(const cv::Point2f  & pMaxDepth, const cv::Point2f  & pMinDepth, const float  & avgDepth, std::vector<float>  & vOridinalDepth); //排除掉坏点，即远离平均值的点//

	void                                                 getValidInnerPoint(const cv::Mat & cv16Img, const std::vector<Point> & srcContours, cv::Point2f & pCenter); //考虑到边界点的深度值不稳定，将边界点的邻域中的内部点替换掉边界点//

	std::vector<cv::Point2f>                             getNeighbour(const cv::Point2f & p);       //获取到某个点的邻域中的点//
	float                                                getAvgPixely(std::vector<cv::Point> & contours);


	//for test//
	void                                                 on_trackbar(int pos, std::vector<vector<Point>> & contours);

	float                                                fOutThreshold;


	void                                                 setPixel(int markpixelx, int markpixely);

	void                                                 ConvertImagePointToHand(const V3f& img, sAction& act, const std::vector<sAction> preHands);  //用于获取手势的姿态//


	//test://
	//用于计算点到平面的距离//
	void                                                 CalDistFromPointToPlane(const V3f &p, const Plane3f & plane, float dist);
	//用于计算点到平面的距离//
	//test://

	//test//
	//讨论主持人在屏幕一侧的情形//
	bool                                                  getSummitPoint(const float  & threshold, const std::vector<cv::Point>  & srcContours, cv::Point  & fSummitPoint, const cv::Mat & cv16img);//若主持人在一侧，通过寻找极端的pixelx,pixely坐标得到手势尖端点的坐标//
	//讨论主持人在屏幕一侧的情形//
	//test//

	bool                                                  IsInOrOutOfZone(const V3f  & KinectPos, const float & fSpeedLength, V3f  vfPospre, V3f vfPosAfter);//判断点是否在边界上//

	int                                                   markpixelx;
	int                                                   markpixely;

	int                                                   iPointthreshold;   //对于有可能出现的，找到的较大值点总在已经找到的边界轮廓附近的情况，使得后面的非需要的边界轮廓点出现在视野中，采取的方法为，设定点的阈值//
	int                                                   depthrevisethreshold;  //当该点的深度和平均值的深度差在阈值以内，我们认为该点的深度是可靠的，可以不需要对深度进行修正//
	float                                                 Radius;               //手势保持静止的半径阈值//
	float                                                 h;                    //手势活动的深度//

	Plane3f                                               pScreenPlane;          //屏幕平面//

	Plane3f                                               pdisScreenPlane;          //离屏平面//

	std::vector<V3f>                                      vdisMarkPoint;           //离屏平面的四个限制点//

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
	//用于判定失去的手势的原因是离开识别区域或者是检测不到引起的HandAction的丢失，flag =true：表示离开识别区域;flag = false表示，由于点未检测到引起的//
	bool                                                  bIsInOrOutFlag;



	float                                                 fActorOnOneSidethreshold; //主持人是否在一侧的阈值//

	cv::Point2f                                           m_CritMarkPoint;        //用于判定手势的运动状态的数据//

	//用于判断匹配的手势是否是同一个手势的阈值//
	std::vector<float>                                    vHandPosThreshold;
	float                                                 fHandPosThreshold;  //用于辅助测试,判断是否是同一个手势://
	//用于判断匹配的手势是否是同一个手势的阈值//

	std::vector <V3f>                                     vfKinectPos;    //为当前手势的KinectPos//


	//用于测试修正的深度是否适合//

	std::vector<cv::Point>                                identPixel;

	std::vector<std::vector<cv::Point3f>>                 vPos;                   //记录前两帧对应的手势的pos//


	//用于获取到当前帧数据与前一帧手势数据中的对应pos//
	bool                                                  getMarchLastHandPos(const std::vector<sAction>  & vLastHand, const cv::Point3f  & kinectPos, const float & threshold, int  & index);



	//for test//

	UINT                                                  getMaxDepth();
	UINT                                                  getMinDepth();

	UINT                                                  iMaxDepth;    //限制区域的最大深度//
	UINT                                                  iMinDepth;    //限制区域的最小深度//

	int                                                   minPixelx;   //屏幕边缘限制的最大最小像素//     //UINT与int类型运算得到结果也是UINT//
	int                                                   maxPixelx;   //屏幕边缘限制的最大最小像素//
	int                                                   minPixely;   //屏幕边缘限制的最大最小像素//
	int                                                   maxPixely;   //屏幕边缘限制的最大最小像素//

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



	//利用K-Means算法剔除掉选出的5个点中的4个得到我们所需要的指尖点//
	cv::Point2f                                          getSummitPoint(std::vector<cv::Point> & contours);
	std::vector<std::vector<cv::Point>>                  getGroup(const int & k, const std::vector<cv::Point>  & srcContours, const float  & threshold);
	void                                                 UpdateGroup(const std::vector<cv::Point> & vMarkPoint, const std::vector<cv::Point>  & srcContours, std::vector<std::vector<cv::Point>> &vGroups);
	float                                                getMin(std::vector<float> vDist);
	cv::Point                                            getOptimalPoint(const std::vector<cv::Point> & srcContours);
	float                                                getAvgDepth(const cv::Mat & cv16Img, std::vector<cv::Point> & contours);
	//利用K-Means算法剔除掉选出的5个点中的4个得到我们所需要的指尖点//
};
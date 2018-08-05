#include "stdafx.h"
#include "CHandTrace.h"
#include "algorithm"


IplImage *g_pGrayImage = NULL;
IplImage *g_pBinaryImage = NULL;

STDMETHODIMP CHandTrace::NonDelegatingQueryInterface(REFIID riid, void **ppv)
{
	if (riid == __uuidof(IHandTrace)) {
		return DLPGetInterface((IHandTrace*)this, ppv);
	}
	return __super::NonDelegatingQueryInterface(riid, ppv);
}

CHandTrace::CHandTrace() 
{
	m_vPixelPointRef.resize(0);
	m_vflag.resize(0);
	m_cv16Img = cv::Mat(m_cDepthHeight, m_cDepthWidth, CV_16UC1);
}

CHandTrace::~CHandTrace()
{
}

void CHandTrace::getKinectPos(const V3f & pos,V3f & Point)
{
	Point.x = (pos[0] - m_cDepthWidth / 2) *pos[2] / foc;
	Point.y = (pos[1] - m_cDepthHeight / 2)  *pos[2] / foc;
	Point.z = pos[2];
}



void CHandTrace::getRatio(const V3f &a, const V3f &b, const V3f & c, V3f &point)
{
	
	float k11 = a.x * a.x + a.y * a.y + a.z * a.z;
	float k12 = a.x * b.x + a.y * b.y + a.z * b.z;
	float k13 = a.x * c.x + a.y * c.y + a.z *c.z;

	float k21 = a.x * b.x + a.y * b.y + a.z * b.z;
	float k22 = b.x * b.x + b.y * b.y + b.z * b.z;
	float k23 = b.x * c.x + b.y * c.y + b.z *c.z;

	point.x = (k13*k22 - k23*k12) / (k11*k22 - k21*k12);
	point.y = (k11*k23 - k21*k13) / (k11*k22 - k21*k12);
}

void  CHandTrace::convertTo(const V3f& pos,V3f & Point)
{
	V3f point1(0,0,0), point2(0,0,0), point3(0,0,0), a(0,0,0), b(0,0,0), c(0,0,0);
	point1 = V3f(m_vKinectPos[0][0], m_vKinectPos[0][1], m_vKinectPos[0][2]);
	point2 = V3f(m_vKinectPos[1][0], m_vKinectPos[1][1], m_vKinectPos[1][2]);
	point3 = V3f(m_vKinectPos[3][0], m_vKinectPos[3][1], m_vKinectPos[3][2]);

	a = point2 - point1;
	b = point3 - point1;
	c = pos - point1;

	V3f K(0, 0, 0);
	getRatio(a, b, c, K);
	V3f point_1, point_2, point_3, a1, a2;
	point_1 = V3f(m_sCreenPos[0][0], m_sCreenPos[0][1], m_sCreenPos[0][2]);
	point_2 = V3f(m_sCreenPos[1][0], m_sCreenPos[1][1], m_sCreenPos[1][2]);
	point_3 = V3f(m_sCreenPos[3][0], m_sCreenPos[3][1], m_sCreenPos[3][2]);
	a1 = point_2 - point_1;
	a2 = point_3 - point_1;
	Point = K.x * a1 + K.y * a2 + point_1;
}


BOOL CHandTrace::InitData(const int &ImgWidth, const int &ImgHeight, const std::vector<nvmath::Vec3f> & vmarkscreenposIn, const std::vector<nvmath::Vec3f> & vmarkKinectPosIn, eInstallMode installMode, float h, float h1, float Radius)
{

	m_cDepthHeight = ImgHeight;
	m_cDepthWidth = ImgWidth;

	//如果标定点的坐标不等于四个返回//
	if (vmarkKinectPosIn.size() != 4 || vmarkscreenposIn.size() != 4)
		return false;

	//给出的是Kinect坐标点，由于深度已知，那可以转化为对应得屏幕像素坐标点//

	m_vpixelKinectPos.resize(vmarkKinectPosIn.size());
	for (int i = 0; i < m_vpixelKinectPos.size(); i++)
	{
		V3f vMarkKinectPos(vmarkKinectPosIn[i][0], vmarkKinectPosIn[i][1], vmarkKinectPosIn[i][2]);
		getPixelPos(vMarkKinectPos, m_vpixelKinectPos[i]);
	}


	m_fHandThreshold = 20;  //设置判断多个手势的阈值，如果大于小于该阈值，则认为是一个手势 删除掉多余的手势//

	if (installMode == Forward)
       m_KinectInstallPattern = 0;

	else 
		m_KinectInstallPattern = 1;
	
	m_fSummitPoint = cv::Point(0, 0); //对手势尖端点赋初值//

	//对手势运动的速度初始化//
	m_fHandPosThreshold = 500;  //判断是否是新手势的标准//
	m_depthrevisethreshold = 80;
	m_fOutThreshold = 10;
	m_iPointthreshold = 30;
	m_fActorOnOneSidethreshold = 20;  //主持人是否在一侧的阈值//

	m_bIsInOrOutFlag = true;

	this->m_h = h;
	this->m_hDistScreen = h1;
	this->m_Radius = Radius;

	m_sCreenPos.resize(vmarkscreenposIn.size());
	m_vKinectPos.resize(vmarkKinectPosIn.size());
	for (int i = 0; i < m_sCreenPos.size(); i++) {
		m_sCreenPos[i] = V3f(vmarkscreenposIn[i][0], vmarkscreenposIn[i][1], vmarkscreenposIn[i][2]);
		m_vKinectPos[i] = V3f(vmarkKinectPosIn[i][0], vmarkKinectPosIn[i][1], vmarkKinectPosIn[i][2]);
	}

	if (min(m_vpixelKinectPos[0].y, m_vpixelKinectPos[3].y) > min(m_vpixelKinectPos[2].y, m_vpixelKinectPos[1].y)) 
		m_minPixely = max(m_vpixelKinectPos[1].y, m_vpixelKinectPos[2].y);
	
	else 
		m_minPixely = max(m_vpixelKinectPos[0].y, m_vpixelKinectPos[3].y);
	

	//获取识别区域的深度以及对应的像素点//
	m_iMinLimitx_1 = min(min(m_vpixelKinectPos[0][0], m_vpixelKinectPos[1][0]), min(m_vpixelKinectPos[2][0], m_vpixelKinectPos[3][0]));
	m_iMaxLimitx_1 = max(max(m_vpixelKinectPos[0][0], m_vpixelKinectPos[1][0]), max(m_vpixelKinectPos[2][0], m_vpixelKinectPos[3][0]));
	m_iMaxLimity_1 = max(max(m_vpixelKinectPos[0][1], m_vpixelKinectPos[1][1]), max(m_vpixelKinectPos[2][1], m_vpixelKinectPos[3][1]));
	m_iMinLimity_1 = min(min(m_vpixelKinectPos[0][1], m_vpixelKinectPos[1][1]), min(m_vpixelKinectPos[2][1], m_vpixelKinectPos[3][1]));
	m_iMaxLimitz_1 = max(max(m_vpixelKinectPos[0][2], m_vpixelKinectPos[1][2]), max(m_vpixelKinectPos[2][2], m_vpixelKinectPos[3][2]));
	m_iMinLimitz_1 = min(min(m_vpixelKinectPos[0][2], m_vpixelKinectPos[1][2]), min(m_vpixelKinectPos[2][2], m_vpixelKinectPos[3][2]));

	for (int i = 0; i < m_vKinectPos.size(); i++) 
	   m_vKinectPos[i] = V3f(vmarkKinectPosIn[i][0], vmarkKinectPosIn[i][1], vmarkKinectPosIn[i][2]);

	//建立Kinect坐标系下的屏幕坐标系//
	SetKinectCoordinateSystem();
	GetInentZone();

	//利用Kinect坐标系下的标定平面坐标对识别区域进行限制//
	m_iMaxLimitx = max(min(m_vKinectPos[0][0], m_vKinectPos[1][0]), min(m_vKinectPos[2][0], m_vKinectPos[3][0]));
	m_iMinLimitx = min(max(m_vKinectPos[0][0], m_vKinectPos[1][0]), max(m_vKinectPos[2][0], m_vKinectPos[3][0]));

	m_iMaxLimitz = max(max(m_vKinectPos[0][2], m_vKinectPos[1][2]), max(m_vKinectPos[2][2], m_vKinectPos[3][2]));
	m_iMinLimitz = min(min(m_vKinectPos[0][2], m_vKinectPos[1][2]), min(m_vKinectPos[2][2], m_vKinectPos[3][2]));

	V3f v1, v2, v3, v4,v_1, v_2, v_3, v_4;

	v1 = m_vKinectPos[0];
	v2 = m_vKinectPos[1];
	v3 = m_vKinectPos[2];
	v4 = m_vKinectPos[3];

	m_pScreenPlane = Plane3f(v1, v2, v3);

	v_1 = v1;
	v_2 = v2;
	v_3 = v3;
	v_4 = v4;

	Plane3f pDistScreen;
	GetPlane(v_1, v_2, v_3, v_4, pDistScreen);

	m_vdisMarkPoint.push_back(v_1);
	m_vdisMarkPoint.push_back(v_2);
	m_vdisMarkPoint.push_back(v_3);
	m_vdisMarkPoint.push_back(v_4);

	m_iMaxDepth = getMaxDepth();
	m_iMinDepth = getMinDepth();
	m_pdisScreenPlane = pDistScreen;


}



UINT  CHandTrace::getMaxDepth() {
	UINT maxDepth = max(max(m_vKinectPos[0][2], m_vKinectPos[1][2]), max(m_vKinectPos[2][2], m_vKinectPos[3][2]));
	return maxDepth;
}

UINT  CHandTrace::getMinDepth() {
	UINT minDepth = min(min(m_vdisMarkPoint[0][2], m_vKinectPos[1][2]), min(m_vKinectPos[2][2], m_vKinectPos[3][2]));
	return minDepth;
}

BOOL CHandTrace::GetHandGesture(const void* pBuffer, int Imagewidth, int Imageheight, std::vector<sAction1> &handAction,INT64 utimastamp)
{
	
	if (m_cv16Img.cols == 0 || m_cv16Img.rows == 0) {

		m_cDepthHeight = Imageheight; m_cDepthWidth = Imagewidth;

		m_cv16Img = cv::Mat(m_cDepthHeight, m_cDepthWidth, CV_16UC1);
		m_cv8Img = cv::Mat(m_cDepthHeight, m_cDepthWidth, CV_8UC1);
	}
	
	for (int i = 0; i < Imageheight; i++) {
		for (int j = 0; j < Imagewidth; j++) {
			m_cv16Img.at<USHORT>(i, j) = *((UINT16*)pBuffer + i*Imagewidth + j);
		}
	}
	Precalc16_fast(m_cv16Img);
	SeekHands(m_cv16Img, handAction);

	for (int i = 0; i < handAction.size(); i++)
	{
		handAction[i].t = utimastamp/10000;
#ifdef DEBUG
		//test system time//
		std::fstream outfile("D:\\systemtime.txt", std::ios::app);
		outfile << handAction[i].t << std::endl;
#endif
	}

	return true;
}

//利用Kinect坐标获取像素点坐标//
void CHandTrace::getPixelPos(const V3f & pos,V3f &pixelPoint) 
{
	pixelPoint.x = pos.x * foc / pos.z + m_cDepthWidth / 2;
	pixelPoint.y = pos.y * foc / pos.z + m_cDepthHeight / 2;
	pixelPoint.z = pos.z;
}

bool CHandTrace::Precalc16_fast(cv::Mat & cv16img)
{
	//test//
#ifdef DEBUG
	static int a = 320;
	static int b = 208;
	float depth = cv16img.at<USHORT>(b, a);
	V3f vtestPixelPos(a, b, 1565);
	V3f vtestKinectPos(0, 0, 0);
	V3f vtestProPos(0, 0, 0);
	getKinectPos(vtestPixelPos, vtestKinectPos);
	getProjectPos(vtestKinectPos, vtestProPos);
	V3f vTransToCalibCoordPoints(0, 0, 0);
	Mult(vtestProPos, vTransToCalibCoordPoints);
#endif

	for (int i = 0; i < m_cDepthHeight; i++) {
		for (int j = 0; j < m_cDepthWidth; j++) {
			V3f pImg;
			pImg.x = j;
			pImg.y = i;
			pImg.z = cv16img.ptr<USHORT>(i)[j];

			//在实际使用中，当Kinect置于屏幕上方时，确定区域的范围是用Kinect的y值来确定的，因此在限定范围的时候，不能用depth来限定//
			//不能利用像素值对图像识别区域边界进行限定，当屏幕离Kinect较近时，可识别的y的区域较小，物体稍微离屏远一点，就会出了屏幕识别区域//
			bool flag = false;
			if (pImg.z> m_iMinLimitz_1 && pImg.z < m_iMaxLimitz_1 && j > m_iMinLimitx_1 && j < m_iMaxLimitx_1) {
				V3f pImgKinectPos(0, 0, 0), projectP(0, 0, 0);
				getKinectPos(pImg, pImgKinectPos);
				getProjectPos(pImgKinectPos, projectP);
				Mult(projectP, m_vProToCalibPlanePos);
				if (m_vProToCalibPlanePos.x > m_fMinIdentX  && m_vProToCalibPlanePos.x < m_fMaxIdentX && m_vProToCalibPlanePos.y > m_fMinIdentY && m_vProToCalibPlanePos.y < m_fMaxIdentY)
					   flag = true;
				
				if (flag) {
					float fdist = (projectP - pImgKinectPos).length();
					if (fdist < m_hDistScreen || fdist > m_h)  
						flag = false;
				}
			}
 
			if (!flag) 
				cv16img.ptr<USHORT>(i)[j] = 0;
		}
	}

	for (int i = 0; i < m_cDepthHeight; i++) {
		for (int j = 0; j < m_cDepthWidth; j++) {
			m_cv8Img.ptr<uchar>(i)[j] = (uchar)(cv16img.at<USHORT>(i, j) * 255.0 / m_iMaxLimitz);
		}
	}

	return  true;
}


bool CHandTrace::IsBetweenTwoPlane(const V3f  & v1, const V3f  &v2, const V3f & v3)
{
	float length1, length2, length3;
	length1 = (v2 - v1).length();
	length2 = (v3 - v2).length();
	length3 = (v3 - v1).length();
	if (length1 < length3 && length2 < length3)
		return true;
	return false;
}

//为简便起见，建立矩形识别框，这样也好确定最大最小深度值，从而确定识别区域//
//建立平面识别区域时，坐标点的顺序应该顺时针给出，这样矩形框的法向量朝上//
void CHandTrace::GetPlane(V3f  & v1, V3f & v2, V3f & v3, V3f & v4, Plane3f &screenPlane1)
{

	V3f vScreen1, vScreen2, vScreen3, vScreen4;
	vScreen1 = v1;
	vScreen2 = v2;
	vScreen3 = v3;
	vScreen4 = v4;
	Plane3f screenPlane(v1, v2, v3);

	V3f normalVector = screenPlane.normal;
	v1 = v1 + m_h * normalVector;
	v2 = v2 + m_h * normalVector;
	v3 = v3 + m_h * normalVector;
	v4 = v4 + m_h * normalVector;
	screenPlane1 =Plane3f(v1, v2, v3);
}

bool CHandTrace::IsInZone(const V3f  & pKinectPos)
{

	//既然是建立矩形识别区域，还是用投影到平面上的坐标较好//

	V3f vfProjectScreen1(0,0,0), vfProjectScreen2(0,0,0);

	getProjectPos(pKinectPos, vfProjectScreen1);
	getProjectPos(pKinectPos, m_pdisScreenPlane, vfProjectScreen2);

	if (IsBetweenTwoPlane(vfProjectScreen1, pKinectPos, vfProjectScreen2))
	{
		if (GetPositionLocation(m_vKinectPos[0], m_vKinectPos[1], m_vKinectPos[2], m_vKinectPos[3], vfProjectScreen1))
		{
			if (GetPositionLocation(m_vdisMarkPoint[0], m_vdisMarkPoint[1], m_vdisMarkPoint[2], m_vdisMarkPoint[3], vfProjectScreen1))
				return true;
		}
	}
	return false;
}

//获取到手势坐标在屏幕平面的投影点，该投影点坐标为Kinect坐标系下的坐标//
void CHandTrace::getProjectPos(const V3f & sKinectPos, V3f & vProjectPoint)
{
	if (sKinectPos.z != 0) {
		Line3f posLine;
		posLine.pos = sKinectPos;
		posLine.dir = m_pScreenPlane.normal;
		m_pScreenPlane.intersect(posLine, vProjectPoint);
	}
}

void CHandTrace::getProjectPos(const V3f & sKinectPos, const Plane3f & plane, V3f &vProjectPoint)
{
	if (sKinectPos.z != 0) {
		Line3f posLine;
		posLine.pos = sKinectPos;
		posLine.dir = plane.normal;
		plane.intersect(posLine, vProjectPoint);
	}
}

bool CHandTrace::GetPositionLocation(const V3f & v1, const V3f & v2, const V3f & v3, const V3f & v4, const V3f & v5)
{
	float angle;
	V3f vVector1 = v2 - v1;
	V3f vVector2 = v3 - v2;
	V3f vVector3 = v4 - v3;
	V3f vVector4 = v1 - v4;
	std::vector<V3f> vEdgeVector;
	vEdgeVector.push_back(vVector1);
	vEdgeVector.push_back(vVector2);
	vEdgeVector.push_back(vVector3);
	vEdgeVector.push_back(vVector4);

	V3f vVector_1 = v5 - v1;
	V3f vVector_2 = v5 - v2;
	V3f vVector_3 = v5 - v3;
	V3f vVector_4 = v5 - v4;
	std::vector<V3f> vPointEdgeVector;
	vPointEdgeVector.push_back(vVector_1);
	vPointEdgeVector.push_back(vVector_2);
	vPointEdgeVector.push_back(vVector_3);
	vPointEdgeVector.push_back(vVector_4);

	std::vector<float>Angle(0);

	angle = acos(((v4 - v1).normalize()).dot((v2 - v1).normalize()));
	Angle.push_back(angle);
	angle = acos(((v3 - v2).normalize()).dot((v1 - v2).normalize()));
	Angle.push_back(angle);
	angle = acos(((v4 - v3).normalize()).dot((v2 - v3).normalize()));
	Angle.push_back(angle);
	angle = acos(((v1 - v4).normalize()).dot((v3 - v4).normalize()));
	Angle.push_back(angle);
	for (int i = 0; i < 4; i++) {
		angle = acos((vEdgeVector[i].normalize()).dot((vPointEdgeVector[i]).normalize()));
		if (angle > Angle[i])
			return false;
	}
	return true;
}

bool CHandTrace::GetHandPos(const Mat & cv16Img, const std::vector<Point>& contour, cv::Point3f  & hand) {

	std::vector<float> vdistance;
	float fInitdis = 100000;
	int  k = 0;



	for (int i = 0; i < contour.size(); i++) {
		cv::Point2i pixelPoint = contour[i];
		USHORT depth = cv16Img.at<USHORT>(pixelPoint.y, pixelPoint.x);



		V3f pKinectCoordinatePos(pixelPoint.x, pixelPoint.y, depth);
		getKinectPos(pKinectCoordinatePos, pKinectCoordinatePos);
		V3f pScreenProjectPos(pKinectCoordinatePos);
		getProjectPos(pScreenProjectPos, pScreenProjectPos);
		V3f del = pScreenProjectPos - pKinectCoordinatePos;
		float fdis = del.length();


		if (fdis < fInitdis)  {
			k = i;
			fInitdis = fdis;
		}

	}
	hand.x = contour[k].x;
	hand.y = contour[k].y;
	hand.z = cv16Img.at<USHORT>(contour[k].y, contour[k].x);
	return true;
}

void CHandTrace::CalcAve(const cv::Mat& cv16img, std::vector<Point>& contour, int  & maxdepth, int & mindepth, float  & avg) {
	int num = (int)contour.size();
	int acc = 0;
	maxdepth = 0;
	mindepth = 10000;


	for (int i = 0; i < num; i++)
	{

		int depth = (int)cv16img.at<ushort>(contour[i].y, contour[i].x);
		if (depth > maxdepth) {
			maxdepth = depth;
		}

		if (depth < mindepth) {
			mindepth = depth;
		}


		acc += (int)cv16img.at<ushort>(contour[i].y, contour[i].x);

		float num1 = num;

		avg = acc / num1;
	}
}

float CHandTrace::CalcAveDepth(const cv::Mat& cv16img, std::vector<Point>& contour)
{

	int   maxdepth = 0, mindepth = 10000;
	float   avg;

	CalcAve(cv16img, contour, maxdepth, mindepth, avg);

	while (abs(maxdepth - avg) > 40 || abs(mindepth - avg) > 40) {

		std::vector<Point> checkContour;
		checkContour.clear();

		if (abs(maxdepth - avg) > abs(mindepth - avg)) {
			if (abs(maxdepth - avg) > 40) {
				int markdepth = maxdepth - 20;
				for (int i = 0; i < contour.size(); i++) {
					if (cv16img.at<ushort>(contour[i].y, contour[i].x) < markdepth) {
						checkContour.push_back(cv::Point(contour[i].x, contour[i].y));
					}
				}
			}
		}

		else {
			if (abs(mindepth - avg) > 40) {
				int markdepth = mindepth + 20;
				for (int i = 0; i < contour.size(); i++) {
					if (cv16img.at<ushort>(contour[i].y, contour[i].x) > markdepth) {
						checkContour.push_back(cv::Point(contour[i].x, contour[i].y));
					}
				}
			}
		}
		contour = checkContour;
		CalcAve(cv16img, contour, maxdepth, mindepth, avg);
	}
	return avg;
}

//对深度值进行修正//
void CHandTrace::depthRevise(cv::Mat & cv16Img, std::vector<Point> & srcContours, std::vector<Point> & contours)
{

	//修正轮廓//

	//test//
	m_vflag.clear();
	m_vflag.resize(contours.size());
	for (int i = 0; i < m_vflag.size(); i++) {
		m_vflag[i] = true;
	}

	for (int i = 0; i < contours.size(); i++)
	{
		//在此间p总是在变化的,需要得到不变的点的深度值//

		//获取替代的像素点//

		cv::Point2f pCenter(0,0);   //存储一个中心标志点//

		std::vector<cv::Point2f> pNeighbour;

		std::vector<cv::Point2f> pCopyNeighbour;

		cv::Point2f p = contours[i];

		//如果这一点的四周都是边界点，将改点置标志位，如果这一点恰好是尖端点，将整个轮廓剃掉//
		BOOL bflag = getValidInnerPointAll(cv16Img, srcContours, p);
		if (!bflag) {
			m_vflag[i] = false;
			break;
		}

		//如果有效，用该内部点替换掉轮廓点//
//		contours[i] = p;

		//获取替代的像素点//

		//内部点的临近方向//
		getNeighbour(p, pNeighbour);
		//内部点的临近方向//

		BOOL bDepthRevise = GetAvgDepthAll(p, srcContours, cv16Img, pNeighbour);

		if (!bflag)
		{
			m_vflag.push_back(i);
			break;
		}
		contours[i]  = p;
	}
}

//通过给定的点，对该点的深度进行修正//

bool CHandTrace::avgDepth(const cv::Point2f & p, const std::vector<Point> & srcContours, cv::Mat & cv16Img)
{
	int Count = 1;
	std::vector<cv::Point3f> vOridinal(0);
	float depth = 0;

	GetValidDepth(p, srcContours, cv16Img, vOridinal,Count,depth);

	if (Count > 4)
	{
		std::vector<float> vOridinalDepth;
		for (int i = 0; i < vOridinal.size(); i++) {
			vOridinalDepth.push_back(vOridinal[i].z);
		}

		cv::Point2f pMaxDepth(0, 0);
	    getMaxDepth(vOridinalDepth, pMaxDepth);
		float fmaxdepth = pMaxDepth.y;
		int iMaxIndex = (int)(pMaxDepth.x);

		cv::Point2f pMinDepth = getMinDepth(vOridinalDepth);
		float fmindepth = pMinDepth.y;
		int iMinIndex = (int)(pMinDepth.x);

		//可以通过bool来判断是否需要继续对vOridinalDepth进行调整： 当bool 为false时,即最大最小值的差在阈值的允许范围内，我们认为这样的数据是好数据，不需要修正，可以用此求得平均值，否则继续对数据进行修正//
		float avgDepth = depth / Count;
		while (elimateBadDepth(pMaxDepth, pMinDepth, avgDepth, vOridinalDepth) && (vOridinalDepth.size() > 2)) {
			getMaxDepth(vOridinalDepth, pMaxDepth);
			pMinDepth = getMinDepth(vOridinalDepth);
			avgDepth = 0;
			for (int i = 0; i < vOridinalDepth.size(); i++) {
				avgDepth += vOridinalDepth[i];
			}
			avgDepth = avgDepth / (vOridinalDepth.size());
		}

		if (vOridinalDepth.size()>2) {

			cv16Img.at<USHORT>(p.y, p.x) = avgDepth;
			return true;
		}

		else {
			if (Count == 1)
				return false;
			else
				if (fabs(vOridinal[0].z - vOridinal[1].z) > 50) {
					return false;
				}

				else {
					cv16Img.at<USHORT>(p.y, p.x) = avgDepth;
					return true;
				}
		}
	}

	else {
		if (Count == 1)
			return false;
		else {
			if (fabs(vOridinal[0].z - vOridinal[0].z) > 70) {
				return false;
			}
			else {
				return true;
			}
		}
	}
}

//对于识别的轮廓，确定给定的点是否是边界点,同时要满足改点为内部点//
bool CHandTrace::isBorderOrOut(const cv::Mat & cv16Img, const std::vector<Point> & contours, const cv::Point & p, const float &fBoundDepth) {

	float fCurrentDepth = cv16Img.at<USHORT>(p.y, p.x);

	if (fCurrentDepth < 10 || fCurrentDepth > fBoundDepth + 50)
		return true;
	for (int i = 0; i < contours.size(); i++) {
		if (p == contours[i])
			return true;
	}
	return false;
}


void CHandTrace::getMaxDepth(const std::vector<float>  & vOridinalDepth,cv::Point2f & Point2f) 
{
	int k = 0;
	float fMaxDepth = 0;
	for (int i = 0; i < vOridinalDepth.size(); i++) {
		if (vOridinalDepth[i]>fMaxDepth) {
			fMaxDepth = vOridinalDepth[i];
			k = i;
		}
	}

	Point2f =  cv::Point2f(k, fMaxDepth);
}

cv::Point2f CHandTrace::getMinDepth(const std::vector<float>  & vOridinalDepth) {
	int k = 0;
	float fMinDepth = 10000;
	for (int i = 0; i < vOridinalDepth.size(); i++) {
		if (vOridinalDepth[i] < fMinDepth) {
			fMinDepth = vOridinalDepth[i];
			k = i;
		}
	}
	return cv::Point2f(k, fMinDepth);
}

//剔掉坏数据的函数//

bool CHandTrace::elimateBadDepth(const cv::Point2f  & pMaxDepth, const cv::Point2f  & pMinDepth, const float  & avgDepth, std::vector<float>  & vOridinalDepth)
{



	std::vector<float>   vCopyOridinalDepth;
	float fmaxdepth = pMaxDepth.y;
	float fmindepth = pMinDepth.y;
	int delIndex = 0;


	if (fabs(fmaxdepth - fmindepth) < 70) {
		return false;
	}

	else {
		if (fabs(fmaxdepth - avgDepth) > fabs(avgDepth - fmindepth)) {
			delIndex = (int)(pMaxDepth.x);
		}
		else {
			delIndex = (int)(pMinDepth.x);
		}

		for (int i = 0; i < vOridinalDepth.size(); i++) {
			if (i != delIndex) {
				vCopyOridinalDepth.push_back(vOridinalDepth[i]);
			}
		}
	}

	vOridinalDepth = vCopyOridinalDepth;
}

//考虑到边界点的深度值不稳定，将边界点的邻域中的内部点替换掉边界点//
BOOL CHandTrace::getValidInnerPoint(const cv::Mat & cv16Img, const std::vector<Point> & srcContours, cv::Point2f & p)
{
	float fBoundDepth = cv16Img.at<USHORT>(p.y, p.x);

	std::vector<Point2f> pneighbour;
	getNeighbour(p, pneighbour);


	int m = 0;    //pneighbour点中的序数//

	//利用内部点替换掉边界点//
	for (m = 0; m < 8; m++) {
		cv::Point pTest = pneighbour[m];

		if (!isBorderOrOut(cv16Img, srcContours, pTest, fBoundDepth)) {
			p = pneighbour[m];
			break;
		}
	}

	return m < 8;

//	assert(m < 8);  //满足的时候不会加显示error，不满足的时候会显示error//

}

//在调用者个函数的前提是必须设定一个阈值，当离平最近的距离点在阈值范围内时候，认为主持人是站在一侧的，可以使用该函数//
//当主持人站在屏幕的一侧的时候，手势可能不会水平，此时利用深度信息不再可行，考虑利用像素点的信息，因为此时所有的像素点位于一侧，而离这一侧距离最远的点近似是手势的尖端点//
//Input：opencv获取的轮廓点//
//output:手势的指尖点//

//获取在一个包中的边缘点的平均深度//
//belong to getSummitPoint//

float CHandTrace::getAvgPixely(std::vector<cv::Point> & contours) {
	float avgPixely = 0;
	for (int i = 0; i < contours.size(); i++) {
		avgPixely = avgPixely + contours[i].y;
	}
	avgPixely = avgPixely / (contours.size());
	return avgPixely;
}

void CHandTrace::on_trackbar(int pos, std::vector<vector<Point>> & contours)
{
	// 转为二值图
	cvThreshold(g_pGrayImage, g_pBinaryImage, pos, 255, CV_THRESH_BINARY);
	// 显示二值图

	cv::Mat MbinaryMat(g_pBinaryImage, 0);

	findContours(MbinaryMat, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);
 
}


//需要知道前一帧的手势来判断当前帧的手势状态,前提是该手势必须在识别区域中//
//对于up点都删掉了，因为已经出去了//
//首先找到当前手势与前一帧的匹配手势；//
//1：如果前一帧无匹配手势，那该手势为down；//
//2:如果前一帧手势为down,利用距离进行判断，如果距离大于阈值,则判断为Move否则为down;  //该距离最好设为屏幕距离//  //
//3:如果前一帧状态为Move,当前帧距离与前一帧距离大于阈值，则仍为Move，否则为none//
//4: 如果前一帧的手势为none，说明前一帧处于等待状态中，如果在屏幕上的移动距离大于Radius,则认为变成Move,否则仍为none//

//5:当手势是Move的时候，由于深度存在误差，以及检测出来的像素也存在误差，因此设定了运动的阈值为25mm，具体的可以根据具体的情况进行改变，当前一帧为None的时候，判断的距离的标准为最近的一个Move的数据，为down时，判断Move的数据也是最近一帧为down的数据//
void CHandTrace::ConvertImagePointToHand(const V3f& p, sAction1& act, const std::vector<sAction1> preHands,const int MarchIndex)
{
	int index = 0;
	V3f pMid(0, 0, 0), pScreenProjectp(0, 0, 0);
	getProjectPos(p, pScreenProjectp);
	convertTo(pScreenProjectp, pMid);

	cv::Point3f pKinectPos = cv::Point3f(p.x, p.y, p.z);
	
	bool  bHasMatchPoint = getMarchLastHandPos(preHands, pKinectPos, m_fHandPosThreshold, index);

	

	if (MarchIndex == -2) {
//	if (!bHasMatchPoint) {
		act.st = Down;
		act.critpos.x = pMid.x;
		act.critpos.y = pMid.y;
		act.critpos.z = 0;
	}

	else if (MarchIndex >= 0)
	{

//	else {

		if (preHands[index].st == Move)
		{
			act.critpos = preHands[index].pos;
		}
		else {
			act.critpos = preHands[index].critpos;
		}


		float distance = sqrt((pMid.x - act.critpos.x) * (pMid.x - act.critpos.x) + (pMid.y - act.critpos.y) * (pMid.y - act.critpos.y));
		if (preHands[index].st == Down) {
			if (distance > m_Radius) {
				act.st = Move;
			}
			else {
				act.st = Down;
			}

		}

		else if (preHands[index].st == Move) {
			if (distance > m_Radius) {
				act.st = Move;
			}
			else {
				act.st = None;
//				act.st = Move;
			}
		}

		else if (preHands[index].st == None){
			if (distance > m_Radius) {
				act.st = Move;
			}
			else {
				act.st = None;
			}
		}
	}


	//获取当前手势的屏幕坐标//
	//如果当前帧对应的手势为Down，前一帧对应的手势也为Down，那当前帧的ScreenPos用前一帧替代，worldPos也用前一帧替代//

	act.pos.x = pMid.x;
	act.pos.y = pMid.y;
	act.pos.z = pMid.z;

	//获取当前手势的全局坐标//
	act.worldPos[0] = p.x;
	act.worldPos[1] = p.y;
	act.worldPos[2] = p.z;

}

//计算点到平面的距离//
void CHandTrace::CalDistFromPointToPlane(const V3f &p, const Plane3f & plane, float dist)
{
	V3f intersetPos;
	V3f dir = plane.normal;
	Line3f line;
	line.dir = dir;
	line.pos = p;
	plane.intersect(line, intersetPos);
	dist = (p - intersetPos).length();
}

//在调用者个函数的前提是必须设定一个阈值，当离平最近的距离点在阈值范围内时候，认为主持人是站在一侧的，可以使用该函数//
//当主持人站在屏幕的一侧的时候，手势可能不会水平，此时利用深度信息不再可行，考虑利用像素点的信息，因为此时所有的像素点位于一侧，而离这一侧距离最远的点近似是手势的尖端点//
//Input：opencv获取的轮廓点//
//output:手势的指尖点//
//这个函数需要重新修正一下，因为Kinect可能会倾斜，不能在Kinect坐标系下用标定板的最小和最大值进行限定，而是应该在标定板坐标系下的最小和最大进行限定//

bool CHandTrace::getSummitPoint(const float  & threshold, const std::vector<cv::Point>  & srcContours, cv::Point  & fSummitPoint, const cv::Mat & cv16img) {


	//需要将srcContours中的点转化成在屏幕坐标的投影的形式，如果存在轮廓点的屏幕上的投影在屏幕边缘，则认为主持人是站在屏幕的一侧的//
	//1：转为屏幕坐标//

	V3f ImgKinectPos(0,0,0), ImgProjectPos(0,0,0), ImgScreenPos(0,0,0) ,  v3fCalibPlanePos(0,0,0);
	std::vector<V3f>  vCalibPlaneCoordinatePos;
	for (int i = 0; i < srcContours.size(); i++) {
		getKinectPos(V3f(srcContours[i].x, srcContours[i].y, cv16img.at<USHORT>(srcContours[i].y, srcContours[i].x)), ImgKinectPos);
		getProjectPos(ImgKinectPos, ImgProjectPos);

		//将图像位置转化到标定板坐标系下//
		Mult(ImgProjectPos, v3fCalibPlanePos);
		vCalibPlaneCoordinatePos.push_back(v3fCalibPlanePos);
		
	}


	//设置标志位//
	enum eDir {
		up,
		down,
		left,
		right,
	};

	eDir esummitdir;
	

	int iMinImgKinectx, iMaxImgKinectx, iMaxImgKinecty, iMinImgKinecty;   //识别的轮廓x和y方向上的最大最小像素坐标点//
	int iMaxIndex, iMinIndex, iMinIndey, iMaxIndey;       //对应x,y方向上最大最小像素点的的指数//

	float fMinDistx = 0, fMaxDistx = 0, fMinDisty = 0, fMaxDisty = 0;

	
	iMinImgKinectx = 10000;
	iMaxImgKinectx = -10000;
	iMaxImgKinecty = -10000;
	iMinImgKinecty = 10000;
	for (int i = 0; i < vCalibPlaneCoordinatePos.size(); i++) {
		//获取在标定板坐标系下像素值x最小的点//
		if (iMinImgKinectx > vCalibPlaneCoordinatePos[i].x) {
			iMinImgKinectx = vCalibPlaneCoordinatePos[i].x;
			iMinIndex = i;
		}


		//获取在标定板坐标系下像素点值x最大的点//
		if (iMaxImgKinectx < vCalibPlaneCoordinatePos[i].x) {
			iMaxImgKinectx = vCalibPlaneCoordinatePos[i].x;
			iMaxIndex = i;
		}

		//获取在标定板坐标系下像素值y最小的点//
		if (iMinImgKinecty > vCalibPlaneCoordinatePos[i].y) {
			iMinImgKinecty = vCalibPlaneCoordinatePos[i].y;
			iMinIndey = i;
		}


		//获取在标定板坐标系下像素点值y最大的点//
		if (iMaxImgKinecty < vCalibPlaneCoordinatePos[i].y) {
			iMaxImgKinecty = vCalibPlaneCoordinatePos[i].y;
			iMaxIndey = i;
		}

	}


	//在标定板上X方向是上下,Y方向上是左右//
	//获取在x像素方向上的最小最大像素点离x边界的距离//
	fMinDistx = abs(m_fMinIdentX - iMinImgKinectx);
	fMaxDistx = abs(m_fMaxIdentX - iMaxImgKinectx);

	//获取标定板y方向最大最小值点距离边界的距离//
	fMinDisty = abs(m_fMinIdentY - iMinImgKinecty);
	fMaxDisty = abs(m_fMaxIdentY - iMaxImgKinecty);




	//与阈值进行判断，确定该函数是否可用，若可用，判断手势的边界条件//
	//规定iMinx:Down;iMaxx:Up;iMiny:Left;iMaxy:Right//
	//如果有一个方向离边界在阈值的范围之内，那对应的方向上的点则为尖端点//

	std::vector<float> vMinDist(0);
	vMinDist.push_back(fMinDistx);  //down//
	vMinDist.push_back(fMaxDistx);  //up//
	vMinDist.push_back(fMinDisty);  //left//
	vMinDist.push_back(fMaxDisty);  //right//

	float fMinDist = vMinDist[0];
	int iGlobalMinIndex = 0;
	for (int i = 1; i < vMinDist.size(); i++) {
		if (vMinDist[i] < fMinDist) {
			fMinDist = vMinDist[i];
			iGlobalMinIndex = i;
		}
	}

	if (threshold < fMinDist) {
		return false;
	}

	switch (iGlobalMinIndex)
	{
	case 0: esummitdir = (eDir)(1); break;
	case 1: esummitdir = (eDir)(0); break;
	case 2: esummitdir = (eDir)(3); break;
	case 3: esummitdir = (eDir)(2); break;
	default: BOOST_ASSERT(!"ERROR DIR"); break;
	}

	
	//与阈值进行判断，确定该函数是否可用，若可用，判断手势的边界条件//




	switch (esummitdir) {
	case left: {
		fSummitPoint = srcContours[iMinIndey];
		break;
	}

	case right: {
		fSummitPoint = srcContours[iMaxIndey];
		break;
	}

	case up: {
		fSummitPoint = srcContours[iMinIndex];
		break;
	}

	case down: {
		fSummitPoint = srcContours[iMaxIndex];
		break;
	}

	default: {
		fSummitPoint = cv::Point2f(0, 0);
		break;
	}
	}

	if (fSummitPoint == cv::Point(0, 0)) {
		return false;
	}

	else {
		return true;
	}
}

//返回true表示在边缘，如果为false表示在识别区域里//
bool CHandTrace::IsInOrOutOfZone(const V3f  & KinectPos, const float & fSpeedLength, V3f  vfPospre, V3f vfPosAfter)
{

	std::vector<float> vPreposDist;
	std::vector<float> vAfterposDist;
	vPreposDist.resize(5);
	vAfterposDist.resize(5);
	std::vector<int> vDir; //当速度朝向该方向时候，对应的数组内的值设为1，否则设为0//
	std::vector<float> fPosDist;
	fPosDist.resize(5);
	//对五个平面进行细分//
	Plane3f p1, p2, p3, p4, p5;

	// m_vKinectPos标记的四个点；vdistMarkPoint:离屏的四个点 
	V3f vMark1, vMark2, vMark3, vMark4, vDistMark1, vDistMark2, vDistMark3, vDistMark4;

	vMark1 = V3f(m_vKinectPos[0].x, m_vKinectPos[0].y, m_vKinectPos[0].z);
	vMark2 = V3f(m_vKinectPos[1].x, m_vKinectPos[1].y, m_vKinectPos[1].z);
	vMark3 = V3f(m_vKinectPos[2].x, m_vKinectPos[2].y, m_vKinectPos[2].z);
	vMark4 = V3f(m_vKinectPos[3].x, m_vKinectPos[3].y, m_vKinectPos[3].z);

	vDistMark1 = V3f(m_vdisMarkPoint[0].x, m_vdisMarkPoint[0].y, m_vdisMarkPoint[0].z);
	vDistMark2 = V3f(m_vdisMarkPoint[1].x, m_vdisMarkPoint[1].y, m_vdisMarkPoint[1].z);
	vDistMark3 = V3f(m_vdisMarkPoint[2].x, m_vdisMarkPoint[2].y, m_vdisMarkPoint[2].z);
	vDistMark4 = V3f(m_vdisMarkPoint[3].x, m_vdisMarkPoint[3].y, m_vdisMarkPoint[3].z);

	p1 = Plane3f(vDistMark1, vDistMark2, vDistMark3);
	p2 = Plane3f(vMark1, vDistMark1, vDistMark4);
	p3 = Plane3f(vMark2, vDistMark2, vMark3);
	p4 = Plane3f(vDistMark3, vDistMark3, vMark4);
	p5 = Plane3f(vMark1, vDistMark1, vMark2);


	std::vector<Plane3f> vPlane(5);

	for (int i = 0; i < vPlane.size(); i++) {
		CalDistFromPointToPlane(KinectPos, vPlane[i], fPosDist[i]);
	}

	for (int i = 0; i < vPlane.size(); i++) {
		CalDistFromPointToPlane(vfPospre, vPlane[i], vPreposDist[i]);
		CalDistFromPointToPlane(vfPosAfter, vPlane[i], vAfterposDist[i]);
		if (vPreposDist[i] > vAfterposDist[i]) {
			vDir.push_back(1);
		}
		else {
			vDir.push_back(0);
		}
	}

	//如果下一个点无法识别到，判断该点是否是因为出了识别区域引起的//

	for (int i = 0; i < vDir.size(); i++) {
		if (vDir[i] == 1) {
			if (fPosDist[i] < m_fOutThreshold) {
				return true;

			}

		}
	}
	return false;
	//如果下一个点无法识别到，判断该点是否是因为出了识别区域引起的//

}

//用于和前一帧的手势进行匹配//
bool  CHandTrace::getMarchLastHandPos(const std::vector<sAction1>  & vLastHand, const cv::Point3f  & kinectPos, const float & threshold, int  & index)
{

	float fMinDist = 10000;
	float fDist;
	int iMinIndex = 0;
	for (int i = 0; i < vLastHand.size(); i++) {
		cv::Point3f pDist = cv::Point3f(vLastHand[i].worldPos[0] - kinectPos.x, vLastHand[i].worldPos[1] - kinectPos.y, vLastHand[i].worldPos[2] - kinectPos.z);
		fDist = sqrt(pDist.x * pDist.x + pDist.y * pDist.y + pDist.z * pDist.z);
		if (fMinDist > fDist) {
			fMinDist = fDist;
			iMinIndex = i;
		}
	}

	if (fMinDist < threshold) {
		index = iMinIndex;
		return true;
	}

	else {
		return false;
	}
}


//根据安装方式的不同，对手势的尖端进行选择//
bool CHandTrace::GetSummitPoint(float a,float b)
{
	if (m_KinectInstallPattern == 0)
	{
		if (a > b)
			return true;
		return false;
	}
	else if (m_KinectInstallPattern == 1) {
		if (a < b)
			return true;
		return false;
	}
}


//利用y值进行判断的缺点是Kinect可能和标定平面不平行，因此用y值进行判断可能会有误点//
//利用离屏的距离，而不是利用y值来进行判断尖端点//
void CHandTrace::getSummitPoint(std::vector<cv::Point> & contours, cv::Point & fsummit) 
{

	if (contours.size() == 1) {
		fsummit = contours[0];
	}


	else {
		std::vector<float> vDistToScreen;
		int minIndex = 0;
		int minDist = 10000;
		for (int i = 0; i < contours.size(); i++) {
			V3f pixelPoint(contours[i].x, contours[i].y, m_cv16Img.at<USHORT>(contours[i].y, contours[i].x));
			V3f KinectPos(0, 0, 0), ImgProjectPos(0, 0, 0);
		    getKinectPos(pixelPoint, KinectPos);
			getProjectPos(KinectPos, ImgProjectPos);
			float fdist = (ImgProjectPos - KinectPos).length();
			if (minDist > fdist) {
				minDist = fdist;
				minIndex = i;
			}
		}
		fsummit =  contours[minIndex];
	}

	
}


//belong to K-Means Algorithm//
//求srcContours点的归属//

void CHandTrace::UpdateGroup(const std::vector<cv::Point> & vMarkPoint, const std::vector<cv::Point>  & srcContours, std::vector<std::vector<cv::Point>> &vGroups) {

	float fMinDist = 10000;
	int iMinIndex = 0;
	float fdist = 0;
	for (int i = 0; i < srcContours.size(); i++) {
		for (int j = 0; j < vMarkPoint.size(); j++) {
			cv::Point2f p((srcContours[i].x - vMarkPoint[j].x), (srcContours[i].y - vMarkPoint[j].y));
			fdist = sqrt(p.x * p.x + p.y * p.y);
			if (fdist < fMinDist) {
				fMinDist = fdist;
				iMinIndex = j;
			}
		}
		fMinDist = 10000;
		vGroups[iMinIndex].push_back(srcContours[i]);
	}
}

//求srcContours点的归属//
//belong to K-Means Algorithm//


float CHandTrace::getMin(std::vector<float> vDist) {

	float fMindist = 10000;
	for (int i = 0; i < vDist.size(); i++) {
		if (vDist[i] < fMindist) {
			fMindist = vDist[i];
		}
	}
	return fMindist;
}

//belong to getSummitPoint//
//获取在一个包中的边缘点的平均深度//
float CHandTrace::getAvgDepth(const cv::Mat & cv16Img, std::vector<cv::Point> & contours) {
	float depth = 0;
	for (int i = 0; i < contours.size(); i++) {
		depth = depth + cv16Img.at<USHORT>(contours[i].y, contours[i].x);
	}
	depth = depth / (contours.size());
	return depth;
}
//获取在一个包中的边缘点的平均深度//
//belong to getSummitPoint//


bool CHandTrace::SetKinectCoordinateSystem()
{
	V3f v1 = m_vKinectPos[0];
	V3f v2 = m_vKinectPos[1];
	V3f v3 = m_vKinectPos[3];
	V3f vRad1 = (v2 - v1).normalize();
	V3f vRad2 = (v3 - v1).normalize();
	V3f vNormVector = (vRad1.cross(vRad2)).normalize();
	Quat<float> q;


	q.setAxisAngle(vNormVector, 1.576786327);
	V3f vAxisY = vRad1 * q;
	V3f vAxisX = vRad1;
	V3f vAxisZ = vNormVector;
	V3f vTransVec = v1;

	Matrix44<float> _invTransMat(1, 0, 0, -vTransVec.x, 0, 1, 0, -vTransVec.y, 0, 0, 1, -vTransVec.z, 0, 0, 0, 1);
	Matrix44<float> _invRotMat(vAxisX.x, vAxisX.y, vAxisX.z, 0, vAxisY.x, vAxisY.y, vAxisY.z, 0, vAxisZ.x, vAxisZ.y, vAxisZ.z, 0, 0, 0, 0, 1);

	m_invTransFromMat = _invRotMat * _invTransMat;
	return true;
}


void CHandTrace::Mult(const V3f &vec,V3f &v)
{
	V4f v1(vec.x,vec.y,vec.z, 1);
	v.x = m_invTransFromMat[0][0] * v1.x + m_invTransFromMat[0][1] * v1.y + m_invTransFromMat[0][2] * v1.z + m_invTransFromMat[0][3] * v1.w;
	v.y = m_invTransFromMat[1][0] * v1.x + m_invTransFromMat[1][1] * v1.y + m_invTransFromMat[1][2] * v1.z + m_invTransFromMat[1][3] * v1.w;
	v.z = m_invTransFromMat[2][0] * v1.x + m_invTransFromMat[2][1] * v1.y + m_invTransFromMat[2][2] * v1.z + m_invTransFromMat[2][3] * v1.w;
}

void CHandTrace::GetInentZone()
{
	float deta = 10;

	V3f v1(0, 0, 0), v2(0, 0, 0), v3(0, 0, 0), v4(0, 0, 0);
	Mult(m_vKinectPos[0], v1);
	Mult(m_vKinectPos[1], v2);
	Mult(m_vKinectPos[2], v3);
	Mult(m_vKinectPos[3], v4);
	m_fMaxIdentX = max(max(v1.x,v2.x), max(v3.x,v4.x))   - deta;
	m_fMinIdentX = min(min(v1.x, v2.x), min(v3.x, v4.x)) + deta;
	m_fMaxIdentY = max(max(v1.y, v2.y), max(v3.y, v4.y)) - deta;
	m_fMinIdentY = min(min(v1.y, v2.y), min(v3.y, v4.y)) + deta;
}


void CHandTrace::GetReviseHandPos()
{
	if (m_vfKinectPos.size() == 0)
		return;
	//减小因为图像识别引起的误差，对于手势中的两个手势距离较近剔除掉多出来的手势  begin://
	for (int i = 0; i < (m_vfKinectPos.size() - 1); i++) {
		for (int j = i + 1; j < m_vfKinectPos.size(); j++) {
			V2f vdistVec = V2f((m_vfKinectPos[j] - m_vfKinectPos[i]).x, (m_vfKinectPos[j] - m_vfKinectPos[i]).y);
			float fdist = vdistVec.length();
			if (fdist < m_fHandThreshold) {
				if (j != m_vfKinectPos.size() - 1) {
					for (int k = j; k < m_vfKinectPos.size() - 1; k++){
						m_vfKinectPos[k] = m_vfKinectPos[k + 1];
					}
				}
				else {
					m_vfKinectPos.pop_back();
				}
			}

		}
	}

	//:end//
}


void CHandTrace::SeekHands(cv::Mat& cv16img,std::vector<sAction1> &hands)
 {

	//对于前一帧是up的情况。不再作为当前帧的参考//
	std::vector<sAction1> vCopyPreHands;
	for (int i = 0; i < m_hands.size(); i++) {
		if (m_hands[i].st != Up ) {
			vCopyPreHands.push_back(m_hands[i]);
		}
	}

	//test vCopyPreHands//
#ifdef DEBUG
	if (vCopyPreHands.size() == 1 && vCopyPreHands[0].ID == 1)
		int testm = 10;
#endif

	m_hands.clear();

	GetCurrentHandsCounter(vCopyPreHands);
//	GetReviseHandPos();  //剔除掉多余的手势//
	adjustPreGesture(vCopyPreHands);

	//上述是对手势轮廓进行检测//
	updateCurrentGesture(vCopyPreHands);

	ReviseHandID(vCopyPreHands);

	hands = m_hands;


#ifdef DEBUG
	if (m_hands.size() == 2)
		if(m_hands[1].st == Move)
			int testm = 10;
#endif

#ifdef DEBUG
	//test//
	if (m_hands.size() >= 2)
		int testm = 10;

	//test//
	for (int i = 0; i < m_hands.size(); i++)
	{
		static int Count = 0;
		if (m_hands[i].st == Up && Count > 0) {
			Count++;
			int testm = 10;
		}
			
	}
#endif


	//test://
#ifdef DEBUG

	if (m_hands.size() > 1) {
		int testm = 10;
	}
#endif


	//test://
	ReviseHands(vCopyPreHands);
	//push m_hands//
	//test//

#ifdef DEBUG
	if (m_hands.size() > 1)
	{
		int testm = 10;
	}

#endif

#ifdef DEBUG
	std::fstream outfile("D:\\myhands1.txt", std::ios::app);
	for (int i = 0; i < m_hands.size(); i++)
	{

		if (m_MyContours.size() <= 2)
		{
			if (m_MyContours.size() == 1)
				m_MyContours.push_back(cv::Point(0, 0));
			else {
				m_MyContours.resize(2);
			}
		}


		if (m_copyMyContours1.size() <= 2)
		{
			if (m_copyMyContours1.size() == 1)
				m_copyMyContours1.push_back(cv::Point(0, 0));
			else {
				m_copyMyContours1.resize(2);
			}
		}
		

	



      //test: get fsummitPoint of each HandGesture//
		V3f pixelPoint(0, 0, 0), kinectpos(m_hands[i].worldPos[0], m_hands[i].worldPos[1], m_hands[i].worldPos[2]);
		getPixelPos(kinectpos, pixelPoint);

		outfile << m_hands[i].ID << " " << m_hands[i].st << " " << m_hands[i].pos.x << " " << m_hands[i].pos.y << " " << m_testfSummitPoint.x << " " << m_testfSummitPoint.y << " " << cv16img.at<USHORT>(pixelPoint.y, pixelPoint.x) <<
			" " << m_MyContours[0].x << " " << m_MyContours[0].y << " " << m_MyContours[1].x << " " << m_MyContours[1].y << "  " << pixelPoint.x << " " << pixelPoint.y << m_copyMyContours[0].x << " " << m_copyMyContours[0].y << " " << m_copyMyContours[1].x << " " << m_copyMyContours[1].y << std::endl;

	}
#endif

	//test for use//
	GetRefPixelPointsAll();

	DeleteUpHands(vCopyPreHands);

//#ifdef DEBUG

	std::fstream outfile("D:\\myhands1.txt", std::ios::app);
	for (int i = 0; i < m_hands.size(); i++) {
		V3f pixelPoint(0, 0, 0), kinectpos(m_hands[i].worldPos[0], m_hands[i].worldPos[1], m_hands[i].worldPos[2]);
		getPixelPos(kinectpos, pixelPoint);
		outfile << m_hands[i].ID << " " << m_hands[i].st << " " << m_hands[i].pos.x << " " << m_hands[i].pos.y << " " << pixelPoint.x << " " << pixelPoint.y << " " << m_hands[i].worldPos[2] << " "<< m_cv16Img.at<USHORT>((int)(pixelPoint.y), (int)(pixelPoint.x)) << std::endl;
	}

//#endif

	if (!m_MyContours.empty())
	    m_MyContours.clear();

	if (!m_contours.empty())
		m_contours.clear();

 	if (!m_MyContours1.empty())
 		m_MyContours1.clear();

	if (!m_vfKinectPos.empty())
		m_vfKinectPos.clear();

	m_fSummitPoint = cv::Point(0, 0);

#ifdef DEBUG
	//test//
	m_testArea = 0;
	m_vcopyMyCounters1.clear();
#endif

}

void CHandTrace::ReviseSummitDepth(const std::vector<sAction1> &vCopyPreHands, cv::Point3f &imgpt)
{

	float PixelDist = 0;
	float fMinPixelDist = 10000;
	int iMinPixelDistIndex = -1;
	V3f p;
	V3f PixelPos;
	for (int i = 0; i < vCopyPreHands.size(); i++) {

		p = V3f(vCopyPreHands[i].worldPos[0], vCopyPreHands[i].worldPos[1], vCopyPreHands[i].worldPos[2]);
		getPixelPos(p, PixelPos);

		//test//
#ifdef DEBUG
		m_PixelPos = PixelPos;
#endif

		PixelDist = abs(m_fSummitPoint.x - PixelPos.x) + abs(m_fSummitPoint.y - PixelPos.y);
		if (PixelDist < fMinPixelDist) {
			fMinPixelDist = PixelDist;
			iMinPixelDistIndex = i;
		}

	}

	if (iMinPixelDistIndex >= 0) {
		p = V3f(vCopyPreHands[iMinPixelDistIndex].worldPos[0], vCopyPreHands[iMinPixelDistIndex].worldPos[1], vCopyPreHands[iMinPixelDistIndex].worldPos[2]);
		getPixelPos(p, PixelPos);
		if (abs(m_fSummitPoint.x - PixelPos.x) < 3 && abs(m_fSummitPoint.y - PixelPos.y) < 3) {
			if (abs(m_fSummitPoint.x - PixelPos.x) + abs(m_fSummitPoint.y - PixelPos.y) < 6) {
				if (fabs(imgpt.z - vCopyPreHands[iMinPixelDistIndex].worldPos[2]) > 50)
				   imgpt.z = vCopyPreHands[iMinPixelDistIndex].worldPos[2];
			}

			if (abs(m_fSummitPoint.x - PixelPos.x) + abs(m_fSummitPoint.y - PixelPos.y) == 0) {
				float fdepthd = fabs(imgpt.z - vCopyPreHands[iMinPixelDistIndex].worldPos[2]);
				if (fdepthd> 50 || fdepthd == 0)
						imgpt.z = vCopyPreHands[iMinPixelDistIndex].worldPos[2] + 1;
			}

		}
	}
}


void CHandTrace::GetCurrentHandsCounter(const std::vector<sAction1> &vCopyPreHands)
{
	//利用二值图获取轮廓//
	g_pGrayImage = &IplImage(m_cv8Img);

	//将灰度图转化成二值图//
	g_pBinaryImage = cvCreateImage(cvGetSize(g_pGrayImage), IPL_DEPTH_8U, 1);
	
	on_trackbar(0, m_contours);

	//test//
#ifdef DEBUG
	std::vector<int> vIndex(0);
	std::vector<float> vArea(0);
#endif

#define HAND_LIKELY_AREA 100
	for (unsigned i = 0; i < m_contours.size(); i++)
	{
		//只有在检测到轮廓时才会去求它的多边形，凸包集，凹陷集
		cv::Mat contour_mat = cv::Mat(m_contours[i]);

		float area = contourArea(contour_mat);

		if (contourArea(contour_mat) > HAND_LIKELY_AREA)
		{
			//test//
#ifdef DEBUG
			vIndex.push_back(i);
			vArea.push_back(area);
#endif
			m_testArea = area;

			//比较有可能像手的区域

			cv::Point3f imgpt;
			cv::Point2f cen; float radius = 0;
			minEnclosingCircle(m_contours[i], cen, radius);
			GetCounterExtremePoint(m_contours[i], cen, radius);

			//获取当前的轮廓点用于获取手势中的轮廓点//
			m_MyContours1 = m_contours[i];

			GetSummitPointAll(vCopyPreHands);


#ifdef DEBUG
			//test:测试轮廓点的深度变化：目的用来判断手势进入时，深度差异为啥这么大//
			std::fstream depthChange("D:\\depthChange.txt", std::ios::app);
			for (int i = 0; i < m_MyContours.size(); i++) {
				depthChange << m_MyContours[i].x << " " << m_MyContours[i].y << " " << m_cv16Img.at<USHORT>(m_MyContours[i].y, m_MyContours[i].x) << std::endl;
			}
			depthChange << m_fSummitPoint.x << " " << m_fSummitPoint.y << " " << m_cv16Img.at<USHORT>(m_MyContours[i].y, m_MyContours[i].x) << std::endl;
			
			depthChange << std::endl;
			depthChange << std::endl;
			depthChange << std::endl;
#endif
			//test:测试轮廓点的深度变化：目的用来判断手势进入时，深度差异为啥这么大//

	

			if (!(m_fSummitPoint == cv::Point(0, 0))) {

				//ps:对于状态丢失的点的状态可以用前面的状态替代//

				imgpt.x = m_fSummitPoint.x;
				imgpt.y = m_fSummitPoint.y;
				imgpt.z = m_cv16Img.at<USHORT>(m_fSummitPoint.y, m_fSummitPoint.x);

				//test : 测试当前帧的深度和前一帧的深度比较，目的：为啥前后两帧的深度差异这么大//
#ifdef DEBUG

				if (vCopyPreHands.size() > 1)
					int testm = 10;
				else if (vCopyPreHands.size() == 1)
				{
					float dist = fabs(imgpt.z - vCopyPreHands[0].worldPos[2]);
					if (dist > 80)
 						int testm = 10;
				}
#endif

				ReviseSummitDepth(vCopyPreHands, imgpt);

				V3f vImgpt;
				vImgpt.x = imgpt.x;
				vImgpt.y = imgpt.y;
				vImgpt.z = imgpt.z;


				//test : 测试当前帧的深度和前一帧的深度比较，目的：为啥前后两帧的深度差异这么大//
				//1:首先进行像素匹配，找到前后两帧对应的像素点，然后如果像素差异较大，则对像度进行修正，说明尖端点找的不好，在像素修正之后，如果深度差异较大，对深度进行修正//
				//如果像素点在差异在20以内，则认为是同一个手势，如果像素点差异在10以外，或者深度差异在50以上，则对像素点进行修正，同时对深度进行修正；//
				//test for use//
				if (!m_vPixelPointRef.empty())
				     fsummitPointRevise(vImgpt);
				//test for use//

				
#ifdef  DEBUG
                //test revise result//
				if (vCopyPreHands.size() > 1)
					int testm = 10;
				else if (vCopyPreHands.size() == 1)
				{
					float dist = fabs(vImgpt.z - vCopyPreHands[0].worldPos[2]);
					if (dist > 80)
						int testm = 10;
				}
				//test revise result//
#endif

				//用于判断手势是否在识别区域内//
				//因为限制区域并不是我们的识别区域，因此在此地还需要判断给出的店是否在识别区域，进行判断，存在这种情况，该点可以被识别到，但是该点不在识别区域中，因此需要对点是否在识别区域进行判断//

				V3f KinectPos(0, 0, 0);
				getKinectPos(vImgpt, KinectPos);
				//获取手势点的Kinectpos,为确定手势点的前两帧服务//
				m_vfKinectPos.push_back(KinectPos);  //有多少只手，vfKinectPos的size就有多大//
			}

			//test//
#ifdef DEBUG
			m_testfSummitPoint = m_fSummitPoint;
#endif

		}

		m_fSummitPoint = cv::Point(0, 0);
	}


#ifdef DEBUG
	//test//
	if (m_vfKinectPos.size() == 1)
		int testm = 10;

	//test//
	if (m_vfKinectPos.size() >= 2 ) {
		int testm = 10;
	}
#endif


#ifdef DEBUG
	static int a = 312;
	static int b = 211;
	float depth = m_cv16Img.at<USHORT>(b, a);
#endif

}

//利用当前帧的历史手势对当前手势进行更新
void CHandTrace::adjustPreGesture(const std::vector<sAction1> &vCopyPreHands)
{
   if (m_vPos.empty()) 
	   InitFrame(vCopyPreHands);

   else
      NonInitFrame(vCopyPreHands);
}




void CHandTrace::updateCurrentGesture(const std::vector<sAction1> &vCopyPreHands)
{

	//************************** need to consider further *************************//
	//获取前两帧的手势的KinectPos//  //有一点需要考虑的是m_hands.size() 这一部分//        //考虑vPos怎么获取到第一个数据的：如果vPos的size为0则利用当前的m_Hands获取第一组数据，如果vPos的size不为0，则利用前一帧和当前帧进行组合获取vPos的数据//

	//vPos的作用：对当前帧的vPos中，如果对应的pos的y,z为-1表示在当前帧是没有该手势的，需要对该手势是丢失还是离开进行判断：方法是利用上一帧的存储的手势的边界阈值//
	//如果是在阈值的范围之外，对该点进行修复，如果在阈值的范围之内，将该手势判定为up//

	//在此对m_hands进行调整//
	//1：如果是在边界上，而且判断为up，则为up//
	//2：:如果是in,则需要修正//
	//3:有一点需要修正的为，当识别不到手势时，存储前三帧的数据，如果前三帧对应的分量的递增和递减的顺序一致，则可以判定不是由于扰动引起的，而是由于Move引起的手势的运动，这时将手势利用速度赋值，否则利用前一帧检测出来的手势赋值

	GetCompensatePos();

	UpdateHand(vCopyPreHands);

}

bool  CHandTrace::GetSummitPoint(const std::vector<sAction1> &vCopyPreHands)
{
	//test: if current pixel is near to the last hand ,it is also a way to ensure the summitpoint  //
	for (int i = 0; i < vCopyPreHands.size(); i++) {
		V3f p = V3f(vCopyPreHands[i].worldPos[0], vCopyPreHands[i].worldPos[1], vCopyPreHands[i].worldPos[2]);
		V3f PixelPos(0, 0, 0);
		getPixelPos(p, PixelPos);
		for (int j = 0; j < m_MyContours.size(); j++) {
			if (fabs(m_MyContours[j].x - PixelPos.x) < 4 && fabs(m_MyContours[j].y - PixelPos.y) < 4) {
				m_fSummitPoint = m_MyContours[j];
				return true;
			}
		}
	}
	return false;
}

void CHandTrace::GetSummitPointAll(const std::vector<sAction1> &vCopyPreHands) {
	
	if (GetSummitPoint(vCopyPreHands)) {
		std::vector<Point>  revisecontours;
		revisecontours.push_back(m_fSummitPoint);
		depthRevise(m_cv16Img, m_MyContours1, revisecontours);

		m_MyContours.clear();

		if (m_fSummitPoint == revisecontours[0]) 
			m_fSummitPoint = cv::Point(0, 0);
		
		else
			m_fSummitPoint = revisecontours[0];

		//如果是通过临近点寻找不到内部点，即使有手势尖端，也将该尖端设置为0//
		if (m_vflag.empty()) {
			if (!m_vflag[0])
				m_fSummitPoint = cv::Point(0, 0);
		}

		//test:if revised//
		float fTestdepth = m_cv16Img.at<USHORT>(m_fSummitPoint.y,m_fSummitPoint.x);
		float fMinValue = 10000;
		int nMinIndex = -1;
		for (int i = 0; i < vCopyPreHands.size(); i++) {
			if (fabs(fTestdepth - vCopyPreHands[i].pos.z) < fMinValue)
				fMinValue = fabs(fTestdepth - vCopyPreHands[i].worldPos[2]);
		}


#ifdef DEBUG
		if (fMinValue > 50 && fMinValue != 10000)
			int testm = 10;
		//test if revised//
#endif

	}

	else  {
		m_copyMyContours = m_MyContours;
		depthRevise(m_cv16Img, m_MyContours1, m_MyContours);

		//测试主持人在屏幕一侧的情形//

		//主持人站在屏幕的一边//   //一般是在左右两边，而不是在上下出去，因此不需要利用像素y进行判断，最好都统一为坐标值来判断//
		//因为在一边时，手势尖端的x像素点总是远离该边，比如如果是左边，即选择对应离左边较远的x像素的点，如果是右边，则选择离右边较远的x像素点对应的点// 

		if (!m_MyContours.empty() && m_fSummitPoint == cv::Point(0,0))
		{
			bool bOneSideflag = getSummitPoint(m_fActorOnOneSidethreshold, m_MyContours, m_fSummitPoint, m_cv16Img);
			bool bInScreenFlag = true;

			//*利用深度调整算法获取到手势尖端点*//

			if (!bOneSideflag) {

				//test//
				//利用K-Means方法获取到手势尖端点//
				if ((m_MyContours.size() > 0)) {
					//test//
					//用于获取到指尖的位置//
					getSummitPoint(m_MyContours, m_fSummitPoint);    //用于获取尖端的像素//
					//用于获取到指尖的位置//
				}
				//利用K-Means方法获取到手势尖端点//
				//test//
			}
		}


#ifdef DEBUG

		//test:if revised//
		float fTestdepth = m_cv16Img.at<USHORT>(m_fSummitPoint.y, m_fSummitPoint.x);
		float fMinValue = 10000;
		int nMinIndex = -1;
		for (int i = 0; i < vCopyPreHands.size(); i++) {
			if (fabs(fTestdepth - vCopyPreHands[i].pos.z) < fMinValue)
				fMinValue = fabs(fTestdepth - vCopyPreHands[i].worldPos[2]);
		}
		if (fMinValue > 50 && fMinValue != 10000 && m_fSummitPoint != cv::Point(0, 0))
			int testm = 10;
		//test if revised//

#endif

#ifdef DEBUG
		if (m_fSummitPoint == cv::Point(0, 0)) {
			int testm = 10;
		}
		//test//
#endif

		//ensure the index of summitPoint in  m_MyContours//
		//test for use//
		int index = -1;
		int MinValue = 10000;
		for (int i = 0; i < m_MyContours.size(); i++) {
			cv::Point2f dpixel = m_MyContours[i] - m_fSummitPoint;
			if (MinValue > sqrt(dpixel.dot(dpixel))) {
				MinValue = sqrt(dpixel.dot(dpixel));
				index = i;
			}
		}
		if (m_vflag.size() > (index + 1))
		{
			if (!m_vflag[index])
				m_fSummitPoint = cv::Point(0, 0);
		}
		//test for use//
		//ensure the index of summitPoint in  m_MyContours//

	}
}



void CHandTrace::GetCounterExtremePoint(const std::vector<cv::Point> &counters, const cv::Point2f &cen, const float &radius)
{
	float dist = 0, dist1 = 0;
	cv::Point pMyPoint;

	std::vector<int> oridinal;

	while (m_MyContours.size() < 5 && (oridinal.size() < m_iPointthreshold)) {
		bool bAddVectorElement = true;
		int imaxindex = 0;
		for (int j = 0; j < counters.size(); j++) {
			int k = 0;
			bool flag = true;


			//剔除已取出的较大dist//
			while (k < oridinal.size()) {
				if (counters[oridinal[k]] == counters[j]) {
					flag = false;
					break;
				}
				k++;
			}

			if (flag) {
				cv::Point p = counters[j];
				dist1 = sqrt((p.x - cen.x)*(p.x - cen.x) + (p.y - cen.y)*(p.y - cen.y));

				if ((dist1 > dist) && (dist1 <= radius)) {
					dist = dist1;
					pMyPoint = p;
					imaxindex = j;

				}
			}
		}


		oridinal.push_back(imaxindex);  //////////////////////////为简便起见，对于存在于MyCouters以及剔除掉的点都放在oridinal中
		dist = 0;
		for (int i = 0; i < m_MyContours.size(); i++) {
			cv::Point pMyContours = m_MyContours[i];
			if ((abs(pMyPoint.x - pMyContours.x) <= 10) && (abs(pMyPoint.y - pMyContours.y) <= 10)) {
				bAddVectorElement = false;
				break;
			}
		}

		//加一个判断语句，如果两点的像素离的比较近，只选择其中一个作为末端像素点//
		if (bAddVectorElement) {
			m_MyContours.push_back(pMyPoint);
		}
	}

#ifdef DEBUG

	//test m_MyContours//
	m_copyMyContours1 = m_MyContours;


	//test//
	m_vcopyMyCounters1.push_back(m_copyMyContours1);
	//test//
	//test m_MyContours//
#endif

}


void CHandTrace::NonInitFrame(const std::vector<sAction1> &vCopyPreHands)
{
	     std::vector<cv::Point3f> vfPos(0);  //用于记录与前一帧ID匹配的pos//

		//先利用固定的阈值进行判断，后续再利用存储的hand点的speed来对阈值进行更新//
		m_vHandPosThreshold.resize(vCopyPreHands.size());
		for (int i = 0; i < vCopyPreHands.size(); i++) {
			m_vHandPosThreshold[i] = m_fHandPosThreshold;
		}
		//先利用固定的阈值进行判断，后续再利用存储的hand点的speed来对阈值进行更新//


		//上一帧和当前帧的pos进行匹配的方法： 首先利用前一帧的手势和当前帧的手势进行比较，获取到当前帧与前一帧手势的点的对应关系，从而可以vidPospush_back(ID/pos),然后对于后面更新的m_Hands将vpos赋予，当前一帧没有手势数据时//
		//可以利用当前的m_Hands自动赋予//

		//***********************如果同时出现手势丢失和有新的手势进来，怎么处理**************************//
		//处理方法：手势丢失或者手势离开的同时有新手势进来：利用阈值进行判定，如果上一帧判定的手势的在屏幕坐标和当前帧的坐标距离差超过阈值的2倍，则认为不是同一个手势，否则认为vCopyPreHands[k]为丢失掉的手势，m_Hands为新增的手势//

		//*************假设已经获取到前一帧手势的阈值:vHandPosThreshold，这是下一步要处理的***************//
		//vHandPosThreshold:有一个默认值，正常情况下的两点之间的距离，同时针对不同的hand运动情况赋予不同的值，现在只考虑常规速度情况，在初始化中设定，后续可以改进//


		//1:前一帧的手势比当前帧的手势要少//
		//解释:是以前一帧的m_Hands为参照物，因此ID是按照前一帧顺下来，所以vPos的对应关系中不需要ID//


		//为了不至于引起混乱，可以先将当前帧的pos调整顺序，最后统一赋予vPos//

  //test//
		//1:前一帧的手势比当前帧的手势少//
      if (vCopyPreHands.size() < m_vfKinectPos.size()) 
	      MatchToLastFrame_1(vCopyPreHands, vfPos);

//2:前一帧的手势比当前帧的手势多//
    else 
         MatchToLastFrame_2(vCopyPreHands, vfPos);


		//利用得到的vfPos对vPos进行更新//
		for (int i = 0; i < vfPos.size(); i++) {
			if (i < m_vPos.size()) {
				if (m_vPos[i].size() < 7) {
					m_vPos[i].push_back(vfPos[i]);
				}

				else {
					int index = -1;
					for (int k = 0; k < m_vPos[i].size() - 1; k++)
					{
						if ((m_vPos[i][k].dot(m_vPos[i][k])) == (m_vPos[i][k + 1].dot(m_vPos[i][k + 1])))
							index = k;
					}
					if (index >= 1) {
						while (index < 6) {
							m_vPos[i][index] = m_vPos[i][index + 1];
							index++;
						}
						m_vPos[i][6] = vfPos[i];

					}

					else {
						m_vPos[i][1] = m_vPos[i][2];
						m_vPos[i][2] = m_vPos[i][3];
						m_vPos[i][3] = m_vPos[i][4];
						m_vPos[i][4] = m_vPos[i][5];
						m_vPos[i][5] = m_vPos[i][6];
						m_vPos[i][6] = vfPos[i];
					}
				}
			}

			else {
				std::vector<cv::Point3f> VNewHands;
				VNewHands.push_back(cv::Point3f(0, 0, 0));
				VNewHands.push_back(vfPos[i]);
				m_vPos.push_back(VNewHands);
			}

		}

		//利用得到的vfPos对vPos进行更新//
}

void CHandTrace::InitFrame(const std::vector<sAction1> &vCopyPreHands) {

	if (m_vfKinectPos.empty())
		return;

	m_vPos.resize(m_vfKinectPos.size());
	for (int i = 0; i < m_vPos.size(); i++) {
		m_vPos[i].push_back(cv::Point3f(0, 0, 0));
		m_vPos[i].push_back(cv::Point3f(m_vfKinectPos[i].x, m_vfKinectPos[i].y, m_vfKinectPos[i].z));
	}
}

//对当前帧比前一帧少的情形下的匹配//
void CHandTrace::MatchToLastFrame_2(const std::vector<sAction1> &vCopyPreHands, std::vector<cv::Point3f> &vfPos)  //用于记录与前一帧ID匹配的pos//)
{
	float fDist = 0;
	float fMinDist = 10000;
	int iMinIndex = 0;
	cv::Point3f pKinectPos(0, 0, 0);
	
	std::vector<int> vMatchIndex(0);


	//初始的vPos应该和前一帧的m_Hand具有相同的size，然后再添加NewHands对vfPos进行增加//
	vfPos.clear();
	vfPos.resize(vCopyPreHands.size());
	//vfPos的初始化假设前一帧的手势点在当前帧都是不存在的//
	for (int i = 0; i < vfPos.size(); i++) {
		vfPos[i] = cv::Point3f(i, -1, -1);
	}


	//步骤1：匹配，找到对应关系//

	//对于当前帧匹配上但是与前一帧的距离大于阈值的手势pos，记录下来赋予新的ID//
	std::vector<cv::Point3f> vCurrentNewHand;

	for (int i = 0; i < m_vfKinectPos.size(); i++) {
		for (int j = 0; j < vCopyPreHands.size(); j++) {

			//已经匹配好的点就不用再匹配了//
			bool bIsMatch = false;
			for (int k = 0; k < vMatchIndex.size(); k++) {
				if (vMatchIndex[k] == j) {
					bIsMatch = true;
					break;
				}
			}
			//已经匹配好的点就不用再匹配了//

			if (!bIsMatch) {
				pKinectPos = cv::Point3f(m_vfKinectPos[i].x, m_vfKinectPos[i].y, m_vfKinectPos[i].z);
				cv::Point3f pPreKinectPos = cv::Point3f(vCopyPreHands[j].worldPos[0], vCopyPreHands[j].worldPos[1], vCopyPreHands[j].worldPos[2]);
				cv::Point3f distp = pKinectPos - pPreKinectPos;
				fDist = sqrt(distp.x * distp.x + distp.y * distp.y + distp.z * distp.z);
				if (fMinDist > fDist) {
					fMinDist = fDist;
					iMinIndex = j;
				}
			}
		}


		//对于大于阈值的2倍情形：前一帧为丢失或者出去的handPos，当前帧对应的hand为新增的handPos,其中需要和对应的Hand的距离阈值进行匹配//
		if (fMinDist > m_vHandPosThreshold[iMinIndex]) 
			vCurrentNewHand.push_back(pKinectPos);                      //记录下此时的m_Hands的pos作为新的手势//
		
		else {
			vfPos[iMinIndex] = cv::Point3f(m_vfKinectPos[i].x, m_vfKinectPos[i].y, m_vfKinectPos[i].z);
			vMatchIndex.push_back(iMinIndex);
		}
			
		fMinDist = 10000;
		iMinIndex = -1;
	}

	//将当前帧与前一帧匹配但是大于阈值的hand的pos赋予vfPos//
	for (int m = 0; m < vCurrentNewHand.size(); m++) {
		vfPos.push_back(vCurrentNewHand[m]);
	}
	//将当前帧与前一帧匹配但是大于阈值的hand的pos赋予vfPos//
}

//当前帧比前一帧多下的匹配//
void CHandTrace::MatchToLastFrame_1(const std::vector<sAction1> &vCopyPreHands, std::vector<cv::Point3f> &vfPos)
{

	float fDist = 0;
	float fMinDist = 1000;
	int iMinIndex = 0;
	cv::Point3f pKinectPos(0, 0, 0);
	std::vector<int> vMatchIndex;

	if (vCopyPreHands.size() < m_vfKinectPos.size()) {
		for (int i = 0; i < vCopyPreHands.size(); i++) {
			for (int j = 0; j < m_vfKinectPos.size(); j++) {

				//已经匹配好的点就不用再匹配了//
				bool bIsMatch = false;
				for (int k = 0; k < vMatchIndex.size(); k++) {
					if (vMatchIndex[k] == j) {
						bIsMatch = true;
						break;
					}
				}
				//已经匹配好的点就不用再匹配了//
				if (!bIsMatch) {
					pKinectPos = cv::Point3f(m_vfKinectPos[j].x, m_vfKinectPos[j].y, m_vfKinectPos[j].z);
					cv::Point3f pPreKinectPos = cv::Point3f(vCopyPreHands[i].worldPos[0], vCopyPreHands[i].worldPos[1], vCopyPreHands[i].worldPos[2]);
					cv::Point3f distp = pKinectPos - pPreKinectPos;
					fDist = sqrt(distp.x * distp.x + distp.y * distp.y + distp.z * distp.z);
					if (fMinDist > fDist) {
						fMinDist = fDist;
						iMinIndex = j;
					}
				}
			}

			//对于大于阈值的2倍情形：前一帧为丢失或者出去的handPos，当前帧对应的hand为新增的handPos,其中需要和对应的Hand的距离阈值进行匹配//
			if (fMinDist > m_vHandPosThreshold[i]) {
				vfPos.push_back(cv::Point3f(i, -1, -1));  //表示对应的前一帧为丢失的关键帧，-1，-1为标志位//
			}

			else {
				vMatchIndex.push_back(iMinIndex);
				vfPos.push_back(cv::Point3f(m_vfKinectPos[iMinIndex].x, m_vfKinectPos[iMinIndex].y, m_vfKinectPos[iMinIndex].z));
			}

			fMinDist = 10000;
			iMinIndex = 0;
		}
		//对于j中未分配的pos直接加到vfPos的后面//

		//利用的思想为：在vMathcIndex中没有的Index则是新多出来的手势，是要被重新分配的//
		for (int k = 0; k < m_vfKinectPos.size(); k++) {
			int l = 0;
			bool flag = true;
			while (l < vMatchIndex.size() && flag) {
				if (k == vMatchIndex[l]) {
					flag = false;
				}
				l++;
			}
			if (flag) {
				vfPos.push_back(cv::Point3f(m_vfKinectPos[k].x, m_vfKinectPos[k].y, m_vfKinectPos[k].z));
			}
		}
		vMatchIndex.clear();   //任何一个vector用完之后一定要clear，防止对其他的程序带来影响//
	}
}

void CHandTrace::GetCompensatePos()
{
	float fSpeedLength = 0;

	V3f pSpeedRef1(0, 0, 0);
	V3f pSpeedRef2(0, 0, 0);
	V3f pSpeedPre(0, 0, 0);
	V3f pSpeedAfter(0, 0, 0);

	for (int i = 0; i < m_vPos.size(); i++) {
		if (m_vPos[i][m_vPos[i].size() - 1].y == -1 && m_vPos[i][m_vPos[i].size() - 1].z == -1) {

			bool bIsInZone = true;

			std::vector<V3f> vLastTwoPos(6);

			if (m_vPos[i].size() < 7) {

				vLastTwoPos[0] = vLastTwoPos[1] = vLastTwoPos[2] = vLastTwoPos[3] = vLastTwoPos[4] = V3f(0, 0, 0);

			}

			else   {
				vLastTwoPos[0] = V3f(m_vPos[i][1].x, m_vPos[i][1].y, m_vPos[i][1].z);
				vLastTwoPos[1] = V3f(m_vPos[i][2].x, m_vPos[i][2].y, m_vPos[i][2].z);
				vLastTwoPos[2] = V3f(m_vPos[i][3].x, m_vPos[i][3].y, m_vPos[i][3].z);
				vLastTwoPos[3] = V3f(m_vPos[i][4].x, m_vPos[i][4].y, m_vPos[i][4].z);
				vLastTwoPos[4] = V3f(m_vPos[i][5].x, m_vPos[i][5].y, m_vPos[i][5].z);
				vLastTwoPos[5] = V3f(m_vPos[i][6].x, m_vPos[i][6].y, m_vPos[i][6].z);
			}

			pSpeedRef1 = vLastTwoPos[2] - vLastTwoPos[1];
			pSpeedRef2 = vLastTwoPos[3] - vLastTwoPos[2];
			pSpeedPre = vLastTwoPos[4] - vLastTwoPos[3];
			pSpeedAfter = vLastTwoPos[5] - vLastTwoPos[4];


			V3f vfPospre(0, 0, 0), vfPosAfter(0, 0, 0);
			if (pSpeedPre.dot(pSpeedAfter) > 0 && pSpeedRef2.dot(pSpeedAfter) > 0 && pSpeedRef2.dot(pSpeedRef1) > 0)
				fSpeedLength = pSpeedAfter.length();
			else
				fSpeedLength = 0;

			if (fSpeedLength == 0)
				m_vPos[i][m_vPos[i].size() - 1] = m_vPos[i][m_vPos[i].size() - 2];

			vfPospre.x = vLastTwoPos[2].x; vfPospre.y = vLastTwoPos[2].y;  vfPospre.z = vLastTwoPos[2].z;
			vfPosAfter.x = vLastTwoPos[3].x; vfPosAfter.y = vLastTwoPos[3].y;  vfPosAfter.z = vLastTwoPos[3].z;

			if (fSpeedLength != 0)
			{
				V3f p(0, 0, 0);
				if (m_vPos[i].size() >= 2)
					p = V3f(m_vPos[i][m_vPos[i].size() - 2].x, m_vPos[i][m_vPos[i].size() - 2].y, m_vPos[i][m_vPos[i].size() - 2].z);

				else {
					p = V3f(m_vPos[i][0].x, m_vPos[i][0].y, m_vPos[i][0].z);
				}

				bIsInZone = IsInOrOutOfZone(p, fSpeedLength, vfPospre, vfPosAfter);

				if (!bIsInZone) {
					//上一帧的数据也识别不到？如果该点没有前一帧的数据，那么该点就根本不存在，所以前一帧的数据肯定是有的//
					m_vPos[i][m_vPos[i].size() - 1] = m_vPos[i][m_vPos[i].size() - 2] + cv::Point3f(pSpeedAfter.x, pSpeedAfter.y, pSpeedAfter.y);
				}
			}

			//true:表示在边缘,且方向是朝着出去的方向,false表示在识别区域中间//
		}
	}
}

void CHandTrace::UpdateHand(const std::vector<sAction1> &vCopyPreHands)
{
	//对当前的m_hand进行更新 ：利用vPos//

	//当前的pos和前一帧的pos的匹配//

	//test//
#ifdef DEBUG
	if (m_vPos.size() == 2)
		int testm = 10;

	if (vCopyPreHands.size() == 2 && vCopyPreHands[1].st == Down)
		int testm = 10;
#endif

	GetMarch(vCopyPreHands,m_vIndex);

	m_hands.resize(m_vPos.size());
	for (int i = 0; i < m_vPos.size(); i++) {
		m_hands[i].ID = i;

		if (m_vPos[i][m_vPos[i].size() - 1].y == -1 && m_vPos[i][m_vPos[i].size() - 1].z == -1) {
			//如果对应的手势为up,设置标志为确定其为up，为下一步删除考虑//
			m_hands[i].st = Up;
			m_hands[i].worldPos[0] = m_vPos[i][m_vPos[i].size() - 2].x;
			m_hands[i].worldPos[1] = m_vPos[i][m_vPos[i].size() - 2].y;
			m_hands[i].worldPos[2] = m_vPos[i][m_vPos[i].size() - 2].z;

			//获取对应的屏幕投影点//
			V3f p = V3f(m_hands[i].worldPos[0], m_hands[i].worldPos[1], m_hands[i].worldPos[2]), pScreenProjectp(0, 0, 0), vD2ScreenProjectp(0, 0, 0);
			getProjectPos(p, pScreenProjectp);
			convertTo(pScreenProjectp, vD2ScreenProjectp);
			m_hands[i].pos.x = vD2ScreenProjectp.x;
			m_hands[i].pos.y = vD2ScreenProjectp.y;
			m_hands[i].pos.z = vD2ScreenProjectp.z;
		}

		else  {
			V3f p = V3f(m_vPos[i][m_vPos[i].size() - 1].x, m_vPos[i][m_vPos[i].size() - 1].y, m_vPos[i][m_vPos[i].size() - 1].z);

			bool bIsInFlag = IsInZone(p);  //用于判断点是否在识别区域内，从而得到该点的状态//
			sAction1 act;
			act.ID = i;
			if (bIsInFlag) {

				ConvertImagePointToHand(p, act, vCopyPreHands,m_vIndex[i]);  //get kinect coordnitate pos//
			}

			else
			{
				act.st = Up;
				V3f pScreenProjectp(0, 0, 0), vD2ScreenProjectp(0, 0, 0);
				getProjectPos(p, pScreenProjectp);
				convertTo(pScreenProjectp, vD2ScreenProjectp);

				act.pos.x = vD2ScreenProjectp.x;
				act.pos.y = vD2ScreenProjectp.y;
				act.pos.z = vD2ScreenProjectp.z;

				act.worldPos[0] = p.x;
				act.worldPos[1] = p.y;
				act.worldPos[2] = p.z;
			}
			m_hands[i] = act;
		}
	}
	m_vIndex.clear();

	//获取到当前帧的手势//

	//对一个手势可能存在的情况是，当前帧手势为up,但是当前帧没有手势，那么该手势总是会作为一个新的手势出现，因此在检测出手势为up后，需要对m_vpos进行相应的更新，删除掉up对应的pos//

	//对手势中，由于与前一帧的数值完全相同的进行清除//
	std::vector<std::vector<cv::Point3f>>   vCopyvPos;
	for (int i = 0; i < m_hands.size(); i++) {
		if (m_hands[i].st != Up) {
			if (m_vPos[i].size() >= 2) {
				if (((m_vPos[i][m_vPos[i].size() - 1]).dot(m_vPos[i][m_vPos[i].size() - 1])) == ((m_vPos[i][m_vPos[i].size() - 2]).dot(m_vPos[i][m_vPos[i].size() - 2])))
				{
					m_vPos[i].pop_back();
					m_vPos[i][0].x++;
					if (m_vPos[i][0].x > 3) {
						m_hands[i].st = Up;
					}

				}
				else
					m_vPos[i][0].x = 0;
			}
			vCopyvPos.push_back(m_vPos[i]);
		}
	}

	m_vPos.clear();
	m_vPos = vCopyvPos;
	for (auto it = m_vPos.begin(); it != m_vPos.end(); ++it) {
		if ((*it)[0].x > 3)
			it = m_vPos.erase(it);
		if (it == m_vPos.end())
			break;
	}

	//test//
#ifdef DEBUG 
	if (m_hands.size() >= 2)
		int testm = 10;
#endif

}


void CHandTrace::ReviseHandID(const std::vector<sAction1> &vCopyPreHands)
{
	

	std::vector<int> vID(vCopyPreHands.size());

	std::vector<int> vMatch(0);
	BOOL bMatch = false;

	for (int i = 0; i < vCopyPreHands.size(); i++){
		vID[i] = vCopyPreHands[i].ID;
	}

	for (int i = 0; i < m_hands.size(); i++) {
		m_hands[i].ID = -1;
		float MinValue = 100000;
		int MinIndex = -1;
		for (int j = 0; j < vCopyPreHands.size(); j++) {

			//已经匹配好的点就不用再匹配了//
			bMatch = false;
			for (int k = 0; k < vMatch.size(); k++) {
				if (vMatch[k] == j) {
					bMatch = true;
					break;
				}
			}

			if (!bMatch) {
				V3f dv(m_hands[i].pos.x - vCopyPreHands[j].pos.x, m_hands[i].pos.y - vCopyPreHands[j].pos.y, m_hands[i].pos.z - vCopyPreHands[j].pos.z);
				if (MinValue > dv.length()) {
					MinIndex = j;
					MinValue = dv.length();
				}
			}

		}

		if (MinValue < m_fHandPosThreshold) {
			m_hands[i].ID = vCopyPreHands[MinIndex].ID;
			vMatch.push_back(MinIndex);
		}

		MinIndex = -1;
		MinValue = 100000;
	}
		int k = 0;
		for (int i = 0; i < m_hands.size(); i++) {
			while (m_hands[i].ID == -1) {
				int j = 0;
					for (; j < vID.size(); j++) {
						if (vID[j] != k)
							continue;
						else
							break;
					}

					if (j == vID.size()) {
						m_hands[i].ID = k;
						vID.push_back(k);
					}
					k++;
			}
		}
}

BOOL CHandTrace::getValidInnerPointAll(const cv::Mat & cv16Img, const std::vector<Point> & srcContours, cv::Point2f & p)
{
	BOOL bflag = (getValidInnerPoint(cv16Img, srcContours, p));

	if (bflag)
		return true;

	std::vector<cv::Point2f> vpNeighbour(0);
	getNeighbour(p, vpNeighbour);

	int k = 0;
	while (!bflag && k < 8) {
		p = vpNeighbour[k];
		bflag = (getValidInnerPoint(cv16Img, srcContours, p));
		k++;
	}
	
	return (k < 8);
}

void CHandTrace::getNeighbour(cv::Point2f & p, std::vector<cv::Point2f>& vNeighbour)
{
	cv::Point2f  pLeft(0,0), pRight(0,0), pUp(0,0), pDown(0,0), pLeftUP(0,0), pLeftDown(0,0), pRightUp(0,0), pRightDown(0,0);

	//邻近方向//
	pLeft = cv::Point2f(p.x - 1, p.y);
	pRight = cv::Point2f(p.x + 1, p.y);
	pUp = cv::Point2f(p.x, p.y - 1);
	pDown = cv::Point2f(p.x, p.y + 1);
	//邻近方向//

	//对角线方向//
	pLeftUP = cv::Point2f(p.x - 1, p.y - 1);
	pLeftDown = cv::Point2f(p.x - 1, p.y + 1);
	pRightUp = cv::Point2f(p.x + 1, p.y - 1);
	pRightDown = cv::Point2f(p.x + 1, p.y + 1);
	//对角线方向//

	vNeighbour.push_back(pLeft);
	vNeighbour.push_back(pRight);
	vNeighbour.push_back(pUp);
	vNeighbour.push_back(pDown);


	//增加四个领域中的点，为获取有效的内部点//
	vNeighbour.push_back(pLeftUP);
	vNeighbour.push_back(pLeftDown);
	vNeighbour.push_back(pRightUp);
	vNeighbour.push_back(pRightDown);
	//增加四个领域中的点，为获取有效的内部点//
}

BOOL CHandTrace::GetAvgDepthAll(cv::Point2f & p, const std::vector<Point> & srcContours, cv::Mat & cv16Img, std::vector<cv::Point2f> &pNeighbour)
{
	bool  bIsavgDepthValid = avgDepth(p, srcContours, cv16Img);

	if (bIsavgDepthValid)
		return true;

	//test:观察深度值是否有变换//
	std::vector<int>vNeighbour;
	vNeighbour.resize(100);

	for (int i = 0; i < vNeighbour.size(); i++) {
		vNeighbour[i] = 0;
	}

	int iLayer = 0;
	cv::Point2f pCenter = p;
	//规定一个标志位，当达到该标志位时，while循环退出，该轮廓点也被设定为无效点//
	int Mark = 0;   //当该标志位到达1,表示进入第三层循环时自动退出循环//

	while (!bIsavgDepthValid && Mark == 0) {
		p = (pNeighbour[vNeighbour[iLayer]]);
		vNeighbour[iLayer] = vNeighbour[iLayer] + 1;

		//如果该点的四周都没有好点//
		if (vNeighbour[iLayer] == 8) {
			if (iLayer > 0) {
				if (vNeighbour[iLayer - 1] == 8) {
					Mark = 1;
				}

				if (Mark == 0) {
					vNeighbour[iLayer] = 0;

					p = pCenter;
					getNeighbour(p, pNeighbour);
					p = pNeighbour[vNeighbour[iLayer - 1]];

					vNeighbour[iLayer - 1] = vNeighbour[iLayer - 1] + 1;

					getNeighbour(p, pNeighbour);
					p = pNeighbour[vNeighbour[iLayer]];
					vNeighbour[iLayer] = vNeighbour[iLayer] + 1;
				}

			}
			else
			{
				vNeighbour[iLayer] = 0;
				iLayer = iLayer + 1;

				p = (pNeighbour[vNeighbour[iLayer - 1]]);
				getNeighbour(p, pNeighbour);
				vNeighbour[iLayer - 1] = vNeighbour[iLayer - 1] + 1;
			}

		}
		//如果该点的四周都没有好点//
		if (Mark == 0) {
			if (p.x > 0 && p.x < m_cDepthWidth && p.y > 0 && p.y < m_cDepthHeight) {
				if (cv16Img.at<USHORT>(p.y, p.x) > 0) {
						bIsavgDepthValid = avgDepth(p, srcContours, cv16Img);
				}
			}

			else {
				bIsavgDepthValid = false;
			}
		}
	}
	//获取该像素点的有效深度值//
	return Mark == 0;

}


void CHandTrace::GetValidDepth(const cv::Point2f & p, const std::vector<Point> & srcContours, cv::Mat & cv16Img,std::vector<cv::Point3f> &vOridinal, int &Count,float& depth)
{
	USHORT depth1 = 0, depth2 = 0, depth3 = 0, depth4 = 0, depth5 = 0, depth6 = 0, depth7 = 0, depth8 = 0, depth9 = 0;

	cv::Point2f  pLeft, pRight, pUp, pDown, pLeftUP, pLeftDown, pRightUp, pRightDown;

	     //for test :在计算平均值的过程中是否存在坏点//

	//test//
	float fBoundDepth = m_cv16Img.at<USHORT>(p.y, p.x);

	pLeft = cv::Point2f(p.x - 1, p.y);
	pRight = cv::Point2f(p.x + 1, p.y);
	pUp = cv::Point2f(p.x, p.y - 1);
	pDown = cv::Point2f(p.x, p.y + 1);


	//对角线方向//
	pLeftUP = cv::Point2f(p.x - 1, p.y - 1);
	pLeftDown = cv::Point2f(p.x - 1, p.y + 1);
	pRightUp = cv::Point2f(p.x + 1, p.y - 1);
	pRightDown = cv::Point2f(p.x + 1, p.y + 1);
	//对角线方向//

	depth1 = cv16Img.at<USHORT>(p.y, p.x);
	depth = depth + depth1;
	vOridinal.push_back(Point3f(p.x, p.y, depth1));

	if (pLeft.x > 0 && pLeft.x < m_cDepthWidth && pLeft.y >0 && pLeft.y < m_cDepthHeight) {
		if (!isBorderOrOut(cv16Img, srcContours, pLeft, fBoundDepth)) {

			depth2 = cv16Img.at<USHORT>(pLeft.y, pLeft.x);
			depth = depth + depth2;
			Count++;
			vOridinal.push_back(Point3f(pLeft.x, pLeft.y, depth2));    //for test :在计算平均值的过程中是否存在坏点//
		}
	}


	if (pRight.x > 0 && pRight.x < m_cDepthWidth && pRight.y >0 && pRight.y < m_cDepthHeight) {
		if (!isBorderOrOut(cv16Img, srcContours, pRight, fBoundDepth)) {

			depth3 = cv16Img.at<USHORT>(pRight.y, pRight.x);
			depth = depth + depth3;
			Count++;

			vOridinal.push_back(Point3f(pRight.x, pRight.y, depth3));  //for test :在计算平均值的过程中是否存在坏点//
		}


	}

	if (pUp.x > 0 && pUp.x < m_cDepthWidth && pUp.y >0 && pUp.y < m_cDepthHeight) {
		if (!isBorderOrOut(cv16Img, srcContours, pUp, fBoundDepth)) {
			depth4 = cv16Img.at<USHORT>(pUp.y, pUp.x);
			depth = depth + depth4;
			Count++;

			vOridinal.push_back(Point3f(pUp.x, pUp.y, depth4));  //for test :在计算平均值的过程中是否存在坏点//
		}
	}

	if (pDown.x > 0 && pDown.x < m_cDepthWidth && pDown.y >0 && pDown.y < m_cDepthHeight) {
		if (!isBorderOrOut(cv16Img, srcContours, pDown, fBoundDepth)) {
			depth5 = cv16Img.at<USHORT>(pDown.y, pDown.x);
			depth = depth + depth5;
			Count++;

			vOridinal.push_back(Point3f(pDown.x, pDown.y, depth5));  //for test :在计算平均值的过程中是否存在坏点//
		}
	}

	//将平均值到8个方向，如果Count较少//   


		if (pLeftUP.x > 0 && pLeftUP.x < m_cDepthWidth && pLeftUP.y >0 && pLeftUP.y < m_cDepthHeight) {
			if (!isBorderOrOut(cv16Img, srcContours, pLeftUP, fBoundDepth)) {
				depth6 = cv16Img.at<USHORT>(pLeftUP.y, pLeftUP.x);
				depth = depth + depth6;
				Count++;
				vOridinal.push_back(Point3f(pLeftUP.x, pLeftUP.y, depth6));  //for test :在计算平均值的过程中是否存在坏点//
			}
		}

		if (pLeftDown.x > 0 && pLeftDown.x < m_cDepthWidth && pLeftDown.y >0 && pLeftDown.y < m_cDepthHeight) {
			if (!isBorderOrOut(cv16Img, srcContours, pLeftDown, fBoundDepth)) {
				depth7 = cv16Img.at<USHORT>(pLeftDown.y, pLeftDown.x);
				depth = depth + depth7;
				Count++;
				vOridinal.push_back(Point3f(pLeftDown.x, pLeftDown.y, depth7)); //for test :在计算平均值的过程中是否存在坏点//
			}
		}

		if (pRightUp.x > 0 && pRightUp.x < m_cDepthWidth && pRightUp.y >0 && pRightUp.y < m_cDepthHeight) {
			if (!isBorderOrOut(cv16Img, srcContours, pRightUp, fBoundDepth)) {
				depth8 = cv16Img.at<USHORT>(pRightUp.y, pRightUp.x);
				depth = depth + depth8;
				Count++;

				vOridinal.push_back(Point3f(pRightUp.x, pRightUp.y, depth8));  //for test :在计算平均值的过程中是否存在坏点//
			}
		}

		if (pRightDown.x > 0 && pRightDown.x < m_cDepthWidth && pRightDown.y >0 && pRightDown.y < m_cDepthHeight) {
			if (!isBorderOrOut(cv16Img, srcContours, pRightDown, fBoundDepth)) {
				depth9 = cv16Img.at<USHORT>(pRightDown.y, pRightDown.x);
				depth = depth + depth9;
				Count++;
				vOridinal.push_back(Point3f(pRightDown.x, pRightDown.y, depth9));  //for test :在计算平均值的过程中是否存在坏点//
			}
		}

	//将平均值到8个方向，如果Count较少//
}

void CHandTrace::deleteHands(const std::vector<sAction1> &vCopyPreHands)
{
	int MaxValue = 0;
	int index    = -1;
	int delIndex = -1;
	for (int i = 0; i < m_hands.size(); i++)
	{
		cv::Point3f  vKinectCurrPos = cv::Point3f(m_hands[i].worldPos[0], m_hands[i].worldPos[1], m_hands[i].worldPos[2]);
		BOOL bMatch = getMarchLastHandPos(vCopyPreHands, vKinectCurrPos, m_fHandThreshold, index);
		if (!bMatch)
		{
			delIndex = i;
			break;
		}
	}

	std::vector<sAction1> copyHands(0);
	for (int i = 0; i < m_hands.size(); i++) {
		if (i != delIndex) {
			copyHands.push_back(m_hands[i]);
		}
	}
	m_hands.clear();
	m_hands = copyHands ;
}

void CHandTrace::ReviseHands(const std::vector<sAction1> &vCopyPreHands)
{
	for (int i = 0; i < m_hands.size(); i++) {
		if (m_hands[i].st == Up) {
			m_testUP = 1;
			m_timebegin = clock();
		}
	}

	if (m_testUP == 1) {

		BOOL bflag = false;

		for (int i = 0; i < m_hands.size(); i++) {
			if (m_hands[i].st == Down) {
				bflag = true;
				break;
			}
		}

		if (bflag) {
			m_timeend = clock();
			if ((m_timeend - m_timebegin) < 100) {
				m_testDown = 1;
				//in this situation delete the more hands///
				if (m_hands.size() > vCopyPreHands.size()) {
					for (int i = 0; i < m_hands.size(); i++) {
						if (vCopyPreHands.size() == 0)
							m_hands.clear();
						else
							deleteHands(vCopyPreHands);
					}
				}
			}
			else
				m_testUP = 0;
		}
	}
}

void CHandTrace::pixelRefMatch()
{
	std::vector<V4f> vpixelPoints(0);
	GetPixelPoints(vpixelPoints);

	for (int i = 0; i < m_hands.size(); i++) {
		int j = 0;
		for (; j < m_vPixelPointRef.size(); j++) {
			if (m_hands[i].ID == m_vPixelPointRef[j][0].x)
				break;
		}

		if (j < m_vPixelPointRef.size()) {
			if (m_vPixelPointRef[j].size() < 3)
				m_vPixelPointRef[j].push_back(vpixelPoints[i]);

			else
			{
				m_vPixelPointRef[j][0] = m_vPixelPointRef[j][1];
				m_vPixelPointRef[j][1] = m_vPixelPointRef[j][2];
				m_vPixelPointRef[j][2] = V4f(vpixelPoints[i]);
			}
		}

		else {
			std::vector<V4f> p;
			p.push_back(V4f(vpixelPoints[i]));
			m_vPixelPointRef.push_back(p);
		}
	}
}


void CHandTrace::GetPixelPoints(std::vector<V4f> &vpixelPoints)
{
	for (int i = 0; i < m_hands.size(); i++) {
		V3f pixelpoint(0, 0, 0); V4f v4pixelPoint(0, 0, 0, 0);
		getPixelPos(V3f(m_hands[i].worldPos[0], m_hands[i].worldPos[1], m_hands[i].worldPos[2]), pixelpoint);
		v4pixelPoint = V4f(m_hands[i].ID, pixelpoint.x, pixelpoint.y, pixelpoint.z);
		vpixelPoints.push_back(v4pixelPoint);
	}
}

void CHandTrace::GetRefPixelPointsAll()
{
	if (m_vPixelPointRef.empty())
	{
		std::vector<V4f> vpixelPoints(0);
		GetPixelPoints(vpixelPoints);
		for (int i = 0; i < m_hands.size(); i++) {
			std::vector<V4f> p;
			p.push_back(vpixelPoints[i]);
			m_vPixelPointRef.push_back(p);
		}
	}

	else {
		std::vector<int> vDelID(0);
		std::vector<std::vector<V4f>> vCopyPre(0);
		for (int i = 0; i < m_vPixelPointRef.size(); i++) {
			int j = 0;
			for (; j < m_hands.size(); j++) {
				if (m_vPixelPointRef[i][0].x == m_hands[j].ID)
					break;
			}
			if (j < m_hands.size())
				vCopyPre.push_back(m_vPixelPointRef[i]);
		}

		m_vPixelPointRef.clear();
		m_vPixelPointRef = vCopyPre;

		pixelRefMatch();
	}
}

BOOL CHandTrace::fsummitPointRevise(V3f &pixelPoint)
{
	int index = -1;
	int MinValue = 10000;
	float dDepth = 10000;
	fsummitPointMarch(pixelPoint.z, index, MinValue, dDepth);
	if (index < 0)
		return false;

	float favg = 0;
	GetPixelRefAvg(index,favg);

	if (MinValue < 20){
		if (MinValue > 10 || dDepth > 60) {
			if (m_vPixelPointRef[index].size() >= 2) {
				int nsize = m_vPixelPointRef[index].size();
				V3f ppre(m_vPixelPointRef[index][nsize - 1].y, m_vPixelPointRef[index][nsize - 1].z, m_vPixelPointRef[index][nsize - 1].w);
				V3f pAfter(m_vPixelPointRef[index][nsize - 2].y, m_vPixelPointRef[index][nsize - 2].z, m_vPixelPointRef[index][nsize - 2].w);

				//分两种情况判断，1：利用前后的速率变换 2：在1的基础上不行时，利用均值判断//
				pixelPoint = (pAfter + (pAfter - ppre) / 2);

				if (fabs(pixelPoint.z - favg) > 80)
					pixelPoint = (pAfter + ppre) / 2;
				return true;
			}
		}
	}
	return false;
}

void CHandTrace::fsummitPointMarch(const float & fcurdepth,int &index, int &MinValue,float & dDepth)
{
	for (int i = 0; i < m_vPixelPointRef.size(); i++) {
		float dpixel = max(fabs(m_fSummitPoint.x - m_vPixelPointRef[i].back().y), fabs(m_fSummitPoint.y - m_vPixelPointRef[i].back().z));
		if (dpixel < MinValue)
		{
			MinValue = dpixel;
			index = i;
		}
	}

	if (index >= 0)
	    dDepth = fabs(fcurdepth  - m_vPixelPointRef[index].back().w);
}

void CHandTrace::GetPixelRefAvg(const int &index,float & favg) 
{

	for (int i = 0; i < m_vPixelPointRef[index].size(); i++)
	{
		favg += m_vPixelPointRef[index][i].w;
	}

	if (favg)
		favg = favg / (m_vPixelPointRef[index].size());
}

//if the first frame is up gesture delete it !! //
void CHandTrace::DeleteUpHands(const std::vector<sAction1> &vCopyPreHands)
{
	std::vector<sAction1> CopyPreHand(0);
	for (int i = 0; i < m_hands.size(); i++) {
		if (m_hands[i].st == Up)  {
			int ID = m_hands[i].ID;
			int j = 0;
			for (; j < vCopyPreHands.size(); j++) {
				if (ID == vCopyPreHands[j].ID)
					break;
			}
			if (j < vCopyPreHands.size())
				CopyPreHand.push_back(m_hands[i]);
		}
		else {
			CopyPreHand.push_back(m_hands[i]);
		}
		
	}
	m_hands.clear();
	m_hands  = CopyPreHand;
}

void CHandTrace::GetMarch(const std::vector<sAction1> &vCopyPreHands, std::vector<int> &vIndex)
{
	vIndex.clear();
	vIndex.resize(m_vPos.size());
	int index = m_vPos.size();

	for (int i = 0; i < vIndex.size(); i++) 
		vIndex[i] = -2;
	
	float MinVaule = 100000;
	float MinIndex = -1;

	V2f vMarchPoint(-1, -1);

	std::vector<sAction1> vCopyPreHands1 = vCopyPreHands;
	std::vector<sAction1> vCopyPreHands2 (0);
	while (index && vCopyPreHands1.size() > 0) {
		for (int i = 0; i < m_vPos.size(); i++) {
			V3f vPos = V3f(m_vPos[i][m_vPos[i].size() - 1].x, m_vPos[i][m_vPos[i].size() - 1].y, m_vPos[i][m_vPos[i].size() - 1].z);
			if (vPos.y == -1 && vPos.z == -1)
			{
				vIndex[i] = -1;
				index--;
				continue;
			}

			if (vIndex[i] != -2)
				continue;
				
			for (int j = 0; j < vCopyPreHands1.size(); j++) {
				V3f del = V3f(vCopyPreHands1[j].worldPos[0] - vPos.x, vCopyPreHands1[j].worldPos[1] - vPos.y, vCopyPreHands1[j].worldPos[2] - vPos.z);
					float fdist = del.length();

					if (MinVaule > fdist) {
						MinVaule = fdist;
						vMarchPoint.x = i;
						vMarchPoint.y = j;
					}	
			}
		}

		MinVaule = -1;
		MinVaule = 100000;

		if (vMarchPoint.x != -1)
		{
			vIndex[vMarchPoint.x] = vMarchPoint.y;
			index--;
		}
		
		
		if (vMarchPoint.y >= 0)
			vCopyPreHands1.erase(vCopyPreHands1.begin() + vMarchPoint.y);
		vMarchPoint = V2f(-1, -1);
// 		for (int k = 0; k < vCopyPreHands1.size(); k++) {
// 			if (k != vMarchPoint.y)
// 				vCopyPreHands2.push_back(vCopyPreHands1[k]);
// 		}

		
// 		vCopyPreHands1.clear();
// 		vCopyPreHands1 = vCopyPreHands2;
// 		vCopyPreHands2.clear();
	}
}

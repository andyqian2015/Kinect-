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

	//����궨������겻�����ĸ�����//
	if (vmarkKinectPosIn.size() != 4 || vmarkscreenposIn.size() != 4)
		return false;

	//��������Kinect����㣬���������֪���ǿ���ת��Ϊ��Ӧ����Ļ���������//

	m_vpixelKinectPos.resize(vmarkKinectPosIn.size());
	for (int i = 0; i < m_vpixelKinectPos.size(); i++)
	{
		V3f vMarkKinectPos(vmarkKinectPosIn[i][0], vmarkKinectPosIn[i][1], vmarkKinectPosIn[i][2]);
		getPixelPos(vMarkKinectPos, m_vpixelKinectPos[i]);
	}


	m_fHandThreshold = 20;  //�����ж϶�����Ƶ���ֵ���������С�ڸ���ֵ������Ϊ��һ������ ɾ�������������//

	if (installMode == Forward)
       m_KinectInstallPattern = 0;

	else 
		m_KinectInstallPattern = 1;
	
	m_fSummitPoint = cv::Point(0, 0); //�����Ƽ�˵㸳��ֵ//

	//�������˶����ٶȳ�ʼ��//
	m_fHandPosThreshold = 500;  //�ж��Ƿ��������Ƶı�׼//
	m_depthrevisethreshold = 80;
	m_fOutThreshold = 10;
	m_iPointthreshold = 30;
	m_fActorOnOneSidethreshold = 20;  //�������Ƿ���һ�����ֵ//

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
	

	//��ȡʶ�����������Լ���Ӧ�����ص�//
	m_iMinLimitx_1 = min(min(m_vpixelKinectPos[0][0], m_vpixelKinectPos[1][0]), min(m_vpixelKinectPos[2][0], m_vpixelKinectPos[3][0]));
	m_iMaxLimitx_1 = max(max(m_vpixelKinectPos[0][0], m_vpixelKinectPos[1][0]), max(m_vpixelKinectPos[2][0], m_vpixelKinectPos[3][0]));
	m_iMaxLimity_1 = max(max(m_vpixelKinectPos[0][1], m_vpixelKinectPos[1][1]), max(m_vpixelKinectPos[2][1], m_vpixelKinectPos[3][1]));
	m_iMinLimity_1 = min(min(m_vpixelKinectPos[0][1], m_vpixelKinectPos[1][1]), min(m_vpixelKinectPos[2][1], m_vpixelKinectPos[3][1]));
	m_iMaxLimitz_1 = max(max(m_vpixelKinectPos[0][2], m_vpixelKinectPos[1][2]), max(m_vpixelKinectPos[2][2], m_vpixelKinectPos[3][2]));
	m_iMinLimitz_1 = min(min(m_vpixelKinectPos[0][2], m_vpixelKinectPos[1][2]), min(m_vpixelKinectPos[2][2], m_vpixelKinectPos[3][2]));

	for (int i = 0; i < m_vKinectPos.size(); i++) 
	   m_vKinectPos[i] = V3f(vmarkKinectPosIn[i][0], vmarkKinectPosIn[i][1], vmarkKinectPosIn[i][2]);

	//����Kinect����ϵ�µ���Ļ����ϵ//
	SetKinectCoordinateSystem();
	GetInentZone();

	//����Kinect����ϵ�µı궨ƽ�������ʶ�������������//
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

//����Kinect�����ȡ���ص�����//
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

			//��ʵ��ʹ���У���Kinect������Ļ�Ϸ�ʱ��ȷ������ķ�Χ����Kinect��yֵ��ȷ���ģ�������޶���Χ��ʱ�򣬲�����depth���޶�//
			//������������ֵ��ͼ��ʶ������߽�����޶�������Ļ��Kinect�Ͻ�ʱ����ʶ���y�������С��������΢����Զһ�㣬�ͻ������Ļʶ������//
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

//Ϊ����������������ʶ�������Ҳ��ȷ�������С���ֵ���Ӷ�ȷ��ʶ������//
//����ƽ��ʶ������ʱ��������˳��Ӧ��˳ʱ��������������ο�ķ���������//
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

	//��Ȼ�ǽ�������ʶ�����򣬻�����ͶӰ��ƽ���ϵ�����Ϻ�//

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

//��ȡ��������������Ļƽ���ͶӰ�㣬��ͶӰ������ΪKinect����ϵ�µ�����//
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

//�����ֵ��������//
void CHandTrace::depthRevise(cv::Mat & cv16Img, std::vector<Point> & srcContours, std::vector<Point> & contours)
{

	//��������//

	//test//
	m_vflag.clear();
	m_vflag.resize(contours.size());
	for (int i = 0; i < m_vflag.size(); i++) {
		m_vflag[i] = true;
	}

	for (int i = 0; i < contours.size(); i++)
	{
		//�ڴ˼�p�����ڱ仯��,��Ҫ�õ�����ĵ�����ֵ//

		//��ȡ��������ص�//

		cv::Point2f pCenter(0,0);   //�洢һ�����ı�־��//

		std::vector<cv::Point2f> pNeighbour;

		std::vector<cv::Point2f> pCopyNeighbour;

		cv::Point2f p = contours[i];

		//�����һ������ܶ��Ǳ߽�㣬���ĵ��ñ�־λ�������һ��ǡ���Ǽ�˵㣬�������������//
		BOOL bflag = getValidInnerPointAll(cv16Img, srcContours, p);
		if (!bflag) {
			m_vflag[i] = false;
			break;
		}

		//�����Ч���ø��ڲ����滻��������//
//		contours[i] = p;

		//��ȡ��������ص�//

		//�ڲ�����ٽ�����//
		getNeighbour(p, pNeighbour);
		//�ڲ�����ٽ�����//

		BOOL bDepthRevise = GetAvgDepthAll(p, srcContours, cv16Img, pNeighbour);

		if (!bflag)
		{
			m_vflag.push_back(i);
			break;
		}
		contours[i]  = p;
	}
}

//ͨ�������ĵ㣬�Ըõ����Ƚ�������//

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

		//����ͨ��bool���ж��Ƿ���Ҫ������vOridinalDepth���е����� ��bool Ϊfalseʱ,�������Сֵ�Ĳ�����ֵ������Χ�ڣ�������Ϊ�����������Ǻ����ݣ�����Ҫ�����������ô����ƽ��ֵ��������������ݽ�������//
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

//����ʶ���������ȷ�������ĵ��Ƿ��Ǳ߽��,ͬʱҪ����ĵ�Ϊ�ڲ���//
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

//�޵������ݵĺ���//

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

//���ǵ��߽������ֵ���ȶ������߽��������е��ڲ����滻���߽��//
BOOL CHandTrace::getValidInnerPoint(const cv::Mat & cv16Img, const std::vector<Point> & srcContours, cv::Point2f & p)
{
	float fBoundDepth = cv16Img.at<USHORT>(p.y, p.x);

	std::vector<Point2f> pneighbour;
	getNeighbour(p, pneighbour);


	int m = 0;    //pneighbour���е�����//

	//�����ڲ����滻���߽��//
	for (m = 0; m < 8; m++) {
		cv::Point pTest = pneighbour[m];

		if (!isBorderOrOut(cv16Img, srcContours, pTest, fBoundDepth)) {
			p = pneighbour[m];
			break;
		}
	}

	return m < 8;

//	assert(m < 8);  //�����ʱ�򲻻����ʾerror���������ʱ�����ʾerror//

}

//�ڵ����߸�������ǰ���Ǳ����趨һ����ֵ������ƽ����ľ��������ֵ��Χ��ʱ����Ϊ��������վ��һ��ģ�����ʹ�øú���//
//��������վ����Ļ��һ���ʱ�����ƿ��ܲ���ˮƽ����ʱ���������Ϣ���ٿ��У������������ص����Ϣ����Ϊ��ʱ���е����ص�λ��һ�࣬������һ�������Զ�ĵ���������Ƶļ�˵�//
//Input��opencv��ȡ��������//
//output:���Ƶ�ָ���//

//��ȡ��һ�����еı�Ե���ƽ�����//
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
	// תΪ��ֵͼ
	cvThreshold(g_pGrayImage, g_pBinaryImage, pos, 255, CV_THRESH_BINARY);
	// ��ʾ��ֵͼ

	cv::Mat MbinaryMat(g_pBinaryImage, 0);

	findContours(MbinaryMat, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);
 
}


//��Ҫ֪��ǰһ֡���������жϵ�ǰ֡������״̬,ǰ���Ǹ����Ʊ�����ʶ��������//
//����up�㶼ɾ���ˣ���Ϊ�Ѿ���ȥ��//
//�����ҵ���ǰ������ǰһ֡��ƥ�����ƣ�//
//1�����ǰһ֡��ƥ�����ƣ��Ǹ�����Ϊdown��//
//2:���ǰһ֡����Ϊdown,���þ�������жϣ�������������ֵ,���ж�ΪMove����Ϊdown;  //�þ��������Ϊ��Ļ����//  //
//3:���ǰһ֡״̬ΪMove,��ǰ֡������ǰһ֡���������ֵ������ΪMove������Ϊnone//
//4: ���ǰһ֡������Ϊnone��˵��ǰһ֡���ڵȴ�״̬�У��������Ļ�ϵ��ƶ��������Radius,����Ϊ���Move,������Ϊnone//

//5:��������Move��ʱ��������ȴ������Լ�������������Ҳ����������趨���˶�����ֵΪ25mm������Ŀ��Ը��ݾ����������иı䣬��ǰһ֡ΪNone��ʱ���жϵľ���ı�׼Ϊ�����һ��Move�����ݣ�Ϊdownʱ���ж�Move������Ҳ�����һ֡Ϊdown������//
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


	//��ȡ��ǰ���Ƶ���Ļ����//
	//�����ǰ֡��Ӧ������ΪDown��ǰһ֡��Ӧ������ҲΪDown���ǵ�ǰ֡��ScreenPos��ǰһ֡�����worldPosҲ��ǰһ֡���//

	act.pos.x = pMid.x;
	act.pos.y = pMid.y;
	act.pos.z = pMid.z;

	//��ȡ��ǰ���Ƶ�ȫ������//
	act.worldPos[0] = p.x;
	act.worldPos[1] = p.y;
	act.worldPos[2] = p.z;

}

//����㵽ƽ��ľ���//
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

//�ڵ����߸�������ǰ���Ǳ����趨һ����ֵ������ƽ����ľ��������ֵ��Χ��ʱ����Ϊ��������վ��һ��ģ�����ʹ�øú���//
//��������վ����Ļ��һ���ʱ�����ƿ��ܲ���ˮƽ����ʱ���������Ϣ���ٿ��У������������ص����Ϣ����Ϊ��ʱ���е����ص�λ��һ�࣬������һ�������Զ�ĵ���������Ƶļ�˵�//
//Input��opencv��ȡ��������//
//output:���Ƶ�ָ���//
//���������Ҫ��������һ�£���ΪKinect���ܻ���б��������Kinect����ϵ���ñ궨�����С�����ֵ�����޶�������Ӧ���ڱ궨������ϵ�µ���С���������޶�//

bool CHandTrace::getSummitPoint(const float  & threshold, const std::vector<cv::Point>  & srcContours, cv::Point  & fSummitPoint, const cv::Mat & cv16img) {


	//��Ҫ��srcContours�еĵ�ת��������Ļ�����ͶӰ����ʽ������������������Ļ�ϵ�ͶӰ����Ļ��Ե������Ϊ��������վ����Ļ��һ���//
	//1��תΪ��Ļ����//

	V3f ImgKinectPos(0,0,0), ImgProjectPos(0,0,0), ImgScreenPos(0,0,0) ,  v3fCalibPlanePos(0,0,0);
	std::vector<V3f>  vCalibPlaneCoordinatePos;
	for (int i = 0; i < srcContours.size(); i++) {
		getKinectPos(V3f(srcContours[i].x, srcContours[i].y, cv16img.at<USHORT>(srcContours[i].y, srcContours[i].x)), ImgKinectPos);
		getProjectPos(ImgKinectPos, ImgProjectPos);

		//��ͼ��λ��ת�����궨������ϵ��//
		Mult(ImgProjectPos, v3fCalibPlanePos);
		vCalibPlaneCoordinatePos.push_back(v3fCalibPlanePos);
		
	}


	//���ñ�־λ//
	enum eDir {
		up,
		down,
		left,
		right,
	};

	eDir esummitdir;
	

	int iMinImgKinectx, iMaxImgKinectx, iMaxImgKinecty, iMinImgKinecty;   //ʶ�������x��y�����ϵ������С���������//
	int iMaxIndex, iMinIndex, iMinIndey, iMaxIndey;       //��Ӧx,y�����������С���ص�ĵ�ָ��//

	float fMinDistx = 0, fMaxDistx = 0, fMinDisty = 0, fMaxDisty = 0;

	
	iMinImgKinectx = 10000;
	iMaxImgKinectx = -10000;
	iMaxImgKinecty = -10000;
	iMinImgKinecty = 10000;
	for (int i = 0; i < vCalibPlaneCoordinatePos.size(); i++) {
		//��ȡ�ڱ궨������ϵ������ֵx��С�ĵ�//
		if (iMinImgKinectx > vCalibPlaneCoordinatePos[i].x) {
			iMinImgKinectx = vCalibPlaneCoordinatePos[i].x;
			iMinIndex = i;
		}


		//��ȡ�ڱ궨������ϵ�����ص�ֵx���ĵ�//
		if (iMaxImgKinectx < vCalibPlaneCoordinatePos[i].x) {
			iMaxImgKinectx = vCalibPlaneCoordinatePos[i].x;
			iMaxIndex = i;
		}

		//��ȡ�ڱ궨������ϵ������ֵy��С�ĵ�//
		if (iMinImgKinecty > vCalibPlaneCoordinatePos[i].y) {
			iMinImgKinecty = vCalibPlaneCoordinatePos[i].y;
			iMinIndey = i;
		}


		//��ȡ�ڱ궨������ϵ�����ص�ֵy���ĵ�//
		if (iMaxImgKinecty < vCalibPlaneCoordinatePos[i].y) {
			iMaxImgKinecty = vCalibPlaneCoordinatePos[i].y;
			iMaxIndey = i;
		}

	}


	//�ڱ궨����X����������,Y������������//
	//��ȡ��x���ط����ϵ���С������ص���x�߽�ľ���//
	fMinDistx = abs(m_fMinIdentX - iMinImgKinectx);
	fMaxDistx = abs(m_fMaxIdentX - iMaxImgKinectx);

	//��ȡ�궨��y���������Сֵ�����߽�ľ���//
	fMinDisty = abs(m_fMinIdentY - iMinImgKinecty);
	fMaxDisty = abs(m_fMaxIdentY - iMaxImgKinecty);




	//����ֵ�����жϣ�ȷ���ú����Ƿ���ã������ã��ж����Ƶı߽�����//
	//�涨iMinx:Down;iMaxx:Up;iMiny:Left;iMaxy:Right//
	//�����һ��������߽�����ֵ�ķ�Χ֮�ڣ��Ƕ�Ӧ�ķ����ϵĵ���Ϊ��˵�//

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

	
	//����ֵ�����жϣ�ȷ���ú����Ƿ���ã������ã��ж����Ƶı߽�����//




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

//����true��ʾ�ڱ�Ե�����Ϊfalse��ʾ��ʶ��������//
bool CHandTrace::IsInOrOutOfZone(const V3f  & KinectPos, const float & fSpeedLength, V3f  vfPospre, V3f vfPosAfter)
{

	std::vector<float> vPreposDist;
	std::vector<float> vAfterposDist;
	vPreposDist.resize(5);
	vAfterposDist.resize(5);
	std::vector<int> vDir; //���ٶȳ���÷���ʱ�򣬶�Ӧ�������ڵ�ֵ��Ϊ1��������Ϊ0//
	std::vector<float> fPosDist;
	fPosDist.resize(5);
	//�����ƽ�����ϸ��//
	Plane3f p1, p2, p3, p4, p5;

	// m_vKinectPos��ǵ��ĸ��㣻vdistMarkPoint:�������ĸ��� 
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

	//�����һ�����޷�ʶ�𵽣��жϸõ��Ƿ�����Ϊ����ʶ�����������//

	for (int i = 0; i < vDir.size(); i++) {
		if (vDir[i] == 1) {
			if (fPosDist[i] < m_fOutThreshold) {
				return true;

			}

		}
	}
	return false;
	//�����һ�����޷�ʶ�𵽣��жϸõ��Ƿ�����Ϊ����ʶ�����������//

}

//���ں�ǰһ֡�����ƽ���ƥ��//
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


//���ݰ�װ��ʽ�Ĳ�ͬ�������Ƶļ�˽���ѡ��//
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


//����yֵ�����жϵ�ȱ����Kinect���ܺͱ궨ƽ�治ƽ�У������yֵ�����жϿ��ܻ������//
//���������ľ��룬����������yֵ�������жϼ�˵�//
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
//��srcContours��Ĺ���//

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

//��srcContours��Ĺ���//
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
//��ȡ��һ�����еı�Ե���ƽ�����//
float CHandTrace::getAvgDepth(const cv::Mat & cv16Img, std::vector<cv::Point> & contours) {
	float depth = 0;
	for (int i = 0; i < contours.size(); i++) {
		depth = depth + cv16Img.at<USHORT>(contours[i].y, contours[i].x);
	}
	depth = depth / (contours.size());
	return depth;
}
//��ȡ��һ�����еı�Ե���ƽ�����//
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
	//��С��Ϊͼ��ʶ������������������е��������ƾ���Ͻ��޳��������������  begin://
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

	//����ǰһ֡��up�������������Ϊ��ǰ֡�Ĳο�//
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
//	GetReviseHandPos();  //�޳������������//
	adjustPreGesture(vCopyPreHands);

	//�����Ƕ������������м��//
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
	//���ö�ֵͼ��ȡ����//
	g_pGrayImage = &IplImage(m_cv8Img);

	//���Ҷ�ͼת���ɶ�ֵͼ//
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
		//ֻ���ڼ�⵽����ʱ�Ż�ȥ�����Ķ���Σ�͹���������ݼ�
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

			//�Ƚ��п������ֵ�����

			cv::Point3f imgpt;
			cv::Point2f cen; float radius = 0;
			minEnclosingCircle(m_contours[i], cen, radius);
			GetCounterExtremePoint(m_contours[i], cen, radius);

			//��ȡ��ǰ�����������ڻ�ȡ�����е�������//
			m_MyContours1 = m_contours[i];

			GetSummitPointAll(vCopyPreHands);


#ifdef DEBUG
			//test:�������������ȱ仯��Ŀ�������ж����ƽ���ʱ����Ȳ���Ϊɶ��ô��//
			std::fstream depthChange("D:\\depthChange.txt", std::ios::app);
			for (int i = 0; i < m_MyContours.size(); i++) {
				depthChange << m_MyContours[i].x << " " << m_MyContours[i].y << " " << m_cv16Img.at<USHORT>(m_MyContours[i].y, m_MyContours[i].x) << std::endl;
			}
			depthChange << m_fSummitPoint.x << " " << m_fSummitPoint.y << " " << m_cv16Img.at<USHORT>(m_MyContours[i].y, m_MyContours[i].x) << std::endl;
			
			depthChange << std::endl;
			depthChange << std::endl;
			depthChange << std::endl;
#endif
			//test:�������������ȱ仯��Ŀ�������ж����ƽ���ʱ����Ȳ���Ϊɶ��ô��//

	

			if (!(m_fSummitPoint == cv::Point(0, 0))) {

				//ps:����״̬��ʧ�ĵ��״̬������ǰ���״̬���//

				imgpt.x = m_fSummitPoint.x;
				imgpt.y = m_fSummitPoint.y;
				imgpt.z = m_cv16Img.at<USHORT>(m_fSummitPoint.y, m_fSummitPoint.x);

				//test : ���Ե�ǰ֡����Ⱥ�ǰһ֡����ȱȽϣ�Ŀ�ģ�Ϊɶǰ����֡����Ȳ�����ô��//
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


				//test : ���Ե�ǰ֡����Ⱥ�ǰһ֡����ȱȽϣ�Ŀ�ģ�Ϊɶǰ����֡����Ȳ�����ô��//
				//1:���Ƚ�������ƥ�䣬�ҵ�ǰ����֡��Ӧ�����ص㣬Ȼ��������ز���ϴ������Ƚ���������˵����˵��ҵĲ��ã�����������֮�������Ȳ���ϴ󣬶���Ƚ�������//
				//������ص��ڲ�����20���ڣ�����Ϊ��ͬһ�����ƣ�������ص������10���⣬������Ȳ�����50���ϣ�������ص����������ͬʱ����Ƚ���������//
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

				//�����ж������Ƿ���ʶ��������//
				//��Ϊ�������򲢲������ǵ�ʶ����������ڴ˵ػ���Ҫ�жϸ����ĵ��Ƿ���ʶ�����򣬽����жϣ���������������õ���Ա�ʶ�𵽣����Ǹõ㲻��ʶ�������У������Ҫ�Ե��Ƿ���ʶ����������ж�//

				V3f KinectPos(0, 0, 0);
				getKinectPos(vImgpt, KinectPos);
				//��ȡ���Ƶ��Kinectpos,Ϊȷ�����Ƶ��ǰ��֡����//
				m_vfKinectPos.push_back(KinectPos);  //�ж���ֻ�֣�vfKinectPos��size���ж��//
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

//���õ�ǰ֡����ʷ���ƶԵ�ǰ���ƽ��и���
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
	//��ȡǰ��֡�����Ƶ�KinectPos//  //��һ����Ҫ���ǵ���m_hands.size() ��һ����//        //����vPos��ô��ȡ����һ�����ݵģ����vPos��sizeΪ0�����õ�ǰ��m_Hands��ȡ��һ�����ݣ����vPos��size��Ϊ0��������ǰһ֡�͵�ǰ֡������ϻ�ȡvPos������//

	//vPos�����ã��Ե�ǰ֡��vPos�У������Ӧ��pos��y,zΪ-1��ʾ�ڵ�ǰ֡��û�и����Ƶģ���Ҫ�Ը������Ƕ�ʧ�����뿪�����жϣ�������������һ֡�Ĵ洢�����Ƶı߽���ֵ//
	//���������ֵ�ķ�Χ֮�⣬�Ըõ�����޸����������ֵ�ķ�Χ֮�ڣ����������ж�Ϊup//

	//�ڴ˶�m_hands���е���//
	//1��������ڱ߽��ϣ������ж�Ϊup����Ϊup//
	//2��:�����in,����Ҫ����//
	//3:��һ����Ҫ������Ϊ����ʶ�𲻵�����ʱ���洢ǰ��֡�����ݣ����ǰ��֡��Ӧ�ķ����ĵ����͵ݼ���˳��һ�£�������ж����������Ŷ�����ģ���������Move��������Ƶ��˶�����ʱ�����������ٶȸ�ֵ����������ǰһ֡�����������Ƹ�ֵ

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

		//�����ͨ���ٽ���Ѱ�Ҳ����ڲ��㣬��ʹ�����Ƽ�ˣ�Ҳ���ü������Ϊ0//
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

		//��������������Ļһ�������//

		//������վ����Ļ��һ��//   //һ�������������ߣ������������³�ȥ����˲���Ҫ��������y�����жϣ���ö�ͳһΪ����ֵ���ж�//
		//��Ϊ��һ��ʱ�����Ƽ�˵�x���ص�����Զ��ñߣ������������ߣ���ѡ���Ӧ����߽�Զ��x���صĵ㣬������ұߣ���ѡ�����ұ߽�Զ��x���ص��Ӧ�ĵ�// 

		if (!m_MyContours.empty() && m_fSummitPoint == cv::Point(0,0))
		{
			bool bOneSideflag = getSummitPoint(m_fActorOnOneSidethreshold, m_MyContours, m_fSummitPoint, m_cv16Img);
			bool bInScreenFlag = true;

			//*������ȵ����㷨��ȡ�����Ƽ�˵�*//

			if (!bOneSideflag) {

				//test//
				//����K-Means������ȡ�����Ƽ�˵�//
				if ((m_MyContours.size() > 0)) {
					//test//
					//���ڻ�ȡ��ָ���λ��//
					getSummitPoint(m_MyContours, m_fSummitPoint);    //���ڻ�ȡ��˵�����//
					//���ڻ�ȡ��ָ���λ��//
				}
				//����K-Means������ȡ�����Ƽ�˵�//
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


			//�޳���ȡ���Ľϴ�dist//
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


		oridinal.push_back(imaxindex);  //////////////////////////Ϊ�����������ڴ�����MyCouters�Լ��޳����ĵ㶼����oridinal��
		dist = 0;
		for (int i = 0; i < m_MyContours.size(); i++) {
			cv::Point pMyContours = m_MyContours[i];
			if ((abs(pMyPoint.x - pMyContours.x) <= 10) && (abs(pMyPoint.y - pMyContours.y) <= 10)) {
				bAddVectorElement = false;
				break;
			}
		}

		//��һ���ж���䣬��������������ıȽϽ���ֻѡ������һ����Ϊĩ�����ص�//
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
	     std::vector<cv::Point3f> vfPos(0);  //���ڼ�¼��ǰһ֡IDƥ���pos//

		//�����ù̶�����ֵ�����жϣ����������ô洢��hand���speed������ֵ���и���//
		m_vHandPosThreshold.resize(vCopyPreHands.size());
		for (int i = 0; i < vCopyPreHands.size(); i++) {
			m_vHandPosThreshold[i] = m_fHandPosThreshold;
		}
		//�����ù̶�����ֵ�����жϣ����������ô洢��hand���speed������ֵ���и���//


		//��һ֡�͵�ǰ֡��pos����ƥ��ķ����� ��������ǰһ֡�����ƺ͵�ǰ֡�����ƽ��бȽϣ���ȡ����ǰ֡��ǰһ֡���Ƶĵ�Ķ�Ӧ��ϵ���Ӷ�����vidPospush_back(ID/pos),Ȼ����ں�����µ�m_Hands��vpos���裬��ǰһ֡û����������ʱ//
		//�������õ�ǰ��m_Hands�Զ�����//

		//***********************���ͬʱ�������ƶ�ʧ�����µ����ƽ�������ô����**************************//
		//�����������ƶ�ʧ���������뿪��ͬʱ�������ƽ�����������ֵ�����ж��������һ֡�ж������Ƶ�����Ļ����͵�ǰ֡�������������ֵ��2��������Ϊ����ͬһ�����ƣ�������ΪvCopyPreHands[k]Ϊ��ʧ�������ƣ�m_HandsΪ����������//

		//*************�����Ѿ���ȡ��ǰһ֡���Ƶ���ֵ:vHandPosThreshold��������һ��Ҫ�����***************//
		//vHandPosThreshold:��һ��Ĭ��ֵ����������µ�����֮��ľ��룬ͬʱ��Բ�ͬ��hand�˶�������費ͬ��ֵ������ֻ���ǳ����ٶ�������ڳ�ʼ�����趨���������ԸĽ�//


		//1:ǰһ֡�����Ʊȵ�ǰ֡������Ҫ��//
		//����:����ǰһ֡��m_HandsΪ��������ID�ǰ���ǰһ֡˳����������vPos�Ķ�Ӧ��ϵ�в���ҪID//


		//Ϊ�˲�����������ң������Ƚ���ǰ֡��pos����˳�����ͳһ����vPos//

  //test//
		//1:ǰһ֡�����Ʊȵ�ǰ֡��������//
      if (vCopyPreHands.size() < m_vfKinectPos.size()) 
	      MatchToLastFrame_1(vCopyPreHands, vfPos);

//2:ǰһ֡�����Ʊȵ�ǰ֡�����ƶ�//
    else 
         MatchToLastFrame_2(vCopyPreHands, vfPos);


		//���õõ���vfPos��vPos���и���//
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

		//���õõ���vfPos��vPos���и���//
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

//�Ե�ǰ֡��ǰһ֡�ٵ������µ�ƥ��//
void CHandTrace::MatchToLastFrame_2(const std::vector<sAction1> &vCopyPreHands, std::vector<cv::Point3f> &vfPos)  //���ڼ�¼��ǰһ֡IDƥ���pos//)
{
	float fDist = 0;
	float fMinDist = 10000;
	int iMinIndex = 0;
	cv::Point3f pKinectPos(0, 0, 0);
	
	std::vector<int> vMatchIndex(0);


	//��ʼ��vPosӦ�ú�ǰһ֡��m_Hand������ͬ��size��Ȼ�������NewHands��vfPos��������//
	vfPos.clear();
	vfPos.resize(vCopyPreHands.size());
	//vfPos�ĳ�ʼ������ǰһ֡�����Ƶ��ڵ�ǰ֡���ǲ����ڵ�//
	for (int i = 0; i < vfPos.size(); i++) {
		vfPos[i] = cv::Point3f(i, -1, -1);
	}


	//����1��ƥ�䣬�ҵ���Ӧ��ϵ//

	//���ڵ�ǰ֡ƥ���ϵ�����ǰһ֡�ľ��������ֵ������pos����¼���������µ�ID//
	std::vector<cv::Point3f> vCurrentNewHand;

	for (int i = 0; i < m_vfKinectPos.size(); i++) {
		for (int j = 0; j < vCopyPreHands.size(); j++) {

			//�Ѿ�ƥ��õĵ�Ͳ�����ƥ����//
			bool bIsMatch = false;
			for (int k = 0; k < vMatchIndex.size(); k++) {
				if (vMatchIndex[k] == j) {
					bIsMatch = true;
					break;
				}
			}
			//�Ѿ�ƥ��õĵ�Ͳ�����ƥ����//

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


		//���ڴ�����ֵ��2�����Σ�ǰһ֡Ϊ��ʧ���߳�ȥ��handPos����ǰ֡��Ӧ��handΪ������handPos,������Ҫ�Ͷ�Ӧ��Hand�ľ�����ֵ����ƥ��//
		if (fMinDist > m_vHandPosThreshold[iMinIndex]) 
			vCurrentNewHand.push_back(pKinectPos);                      //��¼�´�ʱ��m_Hands��pos��Ϊ�µ�����//
		
		else {
			vfPos[iMinIndex] = cv::Point3f(m_vfKinectPos[i].x, m_vfKinectPos[i].y, m_vfKinectPos[i].z);
			vMatchIndex.push_back(iMinIndex);
		}
			
		fMinDist = 10000;
		iMinIndex = -1;
	}

	//����ǰ֡��ǰһ֡ƥ�䵫�Ǵ�����ֵ��hand��pos����vfPos//
	for (int m = 0; m < vCurrentNewHand.size(); m++) {
		vfPos.push_back(vCurrentNewHand[m]);
	}
	//����ǰ֡��ǰһ֡ƥ�䵫�Ǵ�����ֵ��hand��pos����vfPos//
}

//��ǰ֡��ǰһ֡���µ�ƥ��//
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

				//�Ѿ�ƥ��õĵ�Ͳ�����ƥ����//
				bool bIsMatch = false;
				for (int k = 0; k < vMatchIndex.size(); k++) {
					if (vMatchIndex[k] == j) {
						bIsMatch = true;
						break;
					}
				}
				//�Ѿ�ƥ��õĵ�Ͳ�����ƥ����//
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

			//���ڴ�����ֵ��2�����Σ�ǰһ֡Ϊ��ʧ���߳�ȥ��handPos����ǰ֡��Ӧ��handΪ������handPos,������Ҫ�Ͷ�Ӧ��Hand�ľ�����ֵ����ƥ��//
			if (fMinDist > m_vHandPosThreshold[i]) {
				vfPos.push_back(cv::Point3f(i, -1, -1));  //��ʾ��Ӧ��ǰһ֡Ϊ��ʧ�Ĺؼ�֡��-1��-1Ϊ��־λ//
			}

			else {
				vMatchIndex.push_back(iMinIndex);
				vfPos.push_back(cv::Point3f(m_vfKinectPos[iMinIndex].x, m_vfKinectPos[iMinIndex].y, m_vfKinectPos[iMinIndex].z));
			}

			fMinDist = 10000;
			iMinIndex = 0;
		}
		//����j��δ�����posֱ�Ӽӵ�vfPos�ĺ���//

		//���õ�˼��Ϊ����vMathcIndex��û�е�Index�����¶���������ƣ���Ҫ�����·����//
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
		vMatchIndex.clear();   //�κ�һ��vector����֮��һ��Ҫclear����ֹ�������ĳ������Ӱ��//
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
					//��һ֡������Ҳʶ�𲻵�������õ�û��ǰһ֡�����ݣ���ô�õ�͸��������ڣ�����ǰһ֡�����ݿ϶����е�//
					m_vPos[i][m_vPos[i].size() - 1] = m_vPos[i][m_vPos[i].size() - 2] + cv::Point3f(pSpeedAfter.x, pSpeedAfter.y, pSpeedAfter.y);
				}
			}

			//true:��ʾ�ڱ�Ե,�ҷ����ǳ��ų�ȥ�ķ���,false��ʾ��ʶ�������м�//
		}
	}
}

void CHandTrace::UpdateHand(const std::vector<sAction1> &vCopyPreHands)
{
	//�Ե�ǰ��m_hand���и��� ������vPos//

	//��ǰ��pos��ǰһ֡��pos��ƥ��//

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
			//�����Ӧ������Ϊup,���ñ�־Ϊȷ����Ϊup��Ϊ��һ��ɾ������//
			m_hands[i].st = Up;
			m_hands[i].worldPos[0] = m_vPos[i][m_vPos[i].size() - 2].x;
			m_hands[i].worldPos[1] = m_vPos[i][m_vPos[i].size() - 2].y;
			m_hands[i].worldPos[2] = m_vPos[i][m_vPos[i].size() - 2].z;

			//��ȡ��Ӧ����ĻͶӰ��//
			V3f p = V3f(m_hands[i].worldPos[0], m_hands[i].worldPos[1], m_hands[i].worldPos[2]), pScreenProjectp(0, 0, 0), vD2ScreenProjectp(0, 0, 0);
			getProjectPos(p, pScreenProjectp);
			convertTo(pScreenProjectp, vD2ScreenProjectp);
			m_hands[i].pos.x = vD2ScreenProjectp.x;
			m_hands[i].pos.y = vD2ScreenProjectp.y;
			m_hands[i].pos.z = vD2ScreenProjectp.z;
		}

		else  {
			V3f p = V3f(m_vPos[i][m_vPos[i].size() - 1].x, m_vPos[i][m_vPos[i].size() - 1].y, m_vPos[i][m_vPos[i].size() - 1].z);

			bool bIsInFlag = IsInZone(p);  //�����жϵ��Ƿ���ʶ�������ڣ��Ӷ��õ��õ��״̬//
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

	//��ȡ����ǰ֡������//

	//��һ�����ƿ��ܴ��ڵ�����ǣ���ǰ֡����Ϊup,���ǵ�ǰ֡û�����ƣ���ô���������ǻ���Ϊһ���µ����Ƴ��֣�����ڼ�������Ϊup����Ҫ��m_vpos������Ӧ�ĸ��£�ɾ����up��Ӧ��pos//

	//�������У�������ǰһ֡����ֵ��ȫ��ͬ�Ľ������//
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

			//�Ѿ�ƥ��õĵ�Ͳ�����ƥ����//
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

	//�ڽ�����//
	pLeft = cv::Point2f(p.x - 1, p.y);
	pRight = cv::Point2f(p.x + 1, p.y);
	pUp = cv::Point2f(p.x, p.y - 1);
	pDown = cv::Point2f(p.x, p.y + 1);
	//�ڽ�����//

	//�Խ��߷���//
	pLeftUP = cv::Point2f(p.x - 1, p.y - 1);
	pLeftDown = cv::Point2f(p.x - 1, p.y + 1);
	pRightUp = cv::Point2f(p.x + 1, p.y - 1);
	pRightDown = cv::Point2f(p.x + 1, p.y + 1);
	//�Խ��߷���//

	vNeighbour.push_back(pLeft);
	vNeighbour.push_back(pRight);
	vNeighbour.push_back(pUp);
	vNeighbour.push_back(pDown);


	//�����ĸ������еĵ㣬Ϊ��ȡ��Ч���ڲ���//
	vNeighbour.push_back(pLeftUP);
	vNeighbour.push_back(pLeftDown);
	vNeighbour.push_back(pRightUp);
	vNeighbour.push_back(pRightDown);
	//�����ĸ������еĵ㣬Ϊ��ȡ��Ч���ڲ���//
}

BOOL CHandTrace::GetAvgDepthAll(cv::Point2f & p, const std::vector<Point> & srcContours, cv::Mat & cv16Img, std::vector<cv::Point2f> &pNeighbour)
{
	bool  bIsavgDepthValid = avgDepth(p, srcContours, cv16Img);

	if (bIsavgDepthValid)
		return true;

	//test:�۲����ֵ�Ƿ��б任//
	std::vector<int>vNeighbour;
	vNeighbour.resize(100);

	for (int i = 0; i < vNeighbour.size(); i++) {
		vNeighbour[i] = 0;
	}

	int iLayer = 0;
	cv::Point2f pCenter = p;
	//�涨һ����־λ�����ﵽ�ñ�־λʱ��whileѭ���˳�����������Ҳ���趨Ϊ��Ч��//
	int Mark = 0;   //���ñ�־λ����1,��ʾ���������ѭ��ʱ�Զ��˳�ѭ��//

	while (!bIsavgDepthValid && Mark == 0) {
		p = (pNeighbour[vNeighbour[iLayer]]);
		vNeighbour[iLayer] = vNeighbour[iLayer] + 1;

		//����õ�����ܶ�û�кõ�//
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
		//����õ�����ܶ�û�кõ�//
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
	//��ȡ�����ص����Ч���ֵ//
	return Mark == 0;

}


void CHandTrace::GetValidDepth(const cv::Point2f & p, const std::vector<Point> & srcContours, cv::Mat & cv16Img,std::vector<cv::Point3f> &vOridinal, int &Count,float& depth)
{
	USHORT depth1 = 0, depth2 = 0, depth3 = 0, depth4 = 0, depth5 = 0, depth6 = 0, depth7 = 0, depth8 = 0, depth9 = 0;

	cv::Point2f  pLeft, pRight, pUp, pDown, pLeftUP, pLeftDown, pRightUp, pRightDown;

	     //for test :�ڼ���ƽ��ֵ�Ĺ������Ƿ���ڻ���//

	//test//
	float fBoundDepth = m_cv16Img.at<USHORT>(p.y, p.x);

	pLeft = cv::Point2f(p.x - 1, p.y);
	pRight = cv::Point2f(p.x + 1, p.y);
	pUp = cv::Point2f(p.x, p.y - 1);
	pDown = cv::Point2f(p.x, p.y + 1);


	//�Խ��߷���//
	pLeftUP = cv::Point2f(p.x - 1, p.y - 1);
	pLeftDown = cv::Point2f(p.x - 1, p.y + 1);
	pRightUp = cv::Point2f(p.x + 1, p.y - 1);
	pRightDown = cv::Point2f(p.x + 1, p.y + 1);
	//�Խ��߷���//

	depth1 = cv16Img.at<USHORT>(p.y, p.x);
	depth = depth + depth1;
	vOridinal.push_back(Point3f(p.x, p.y, depth1));

	if (pLeft.x > 0 && pLeft.x < m_cDepthWidth && pLeft.y >0 && pLeft.y < m_cDepthHeight) {
		if (!isBorderOrOut(cv16Img, srcContours, pLeft, fBoundDepth)) {

			depth2 = cv16Img.at<USHORT>(pLeft.y, pLeft.x);
			depth = depth + depth2;
			Count++;
			vOridinal.push_back(Point3f(pLeft.x, pLeft.y, depth2));    //for test :�ڼ���ƽ��ֵ�Ĺ������Ƿ���ڻ���//
		}
	}


	if (pRight.x > 0 && pRight.x < m_cDepthWidth && pRight.y >0 && pRight.y < m_cDepthHeight) {
		if (!isBorderOrOut(cv16Img, srcContours, pRight, fBoundDepth)) {

			depth3 = cv16Img.at<USHORT>(pRight.y, pRight.x);
			depth = depth + depth3;
			Count++;

			vOridinal.push_back(Point3f(pRight.x, pRight.y, depth3));  //for test :�ڼ���ƽ��ֵ�Ĺ������Ƿ���ڻ���//
		}


	}

	if (pUp.x > 0 && pUp.x < m_cDepthWidth && pUp.y >0 && pUp.y < m_cDepthHeight) {
		if (!isBorderOrOut(cv16Img, srcContours, pUp, fBoundDepth)) {
			depth4 = cv16Img.at<USHORT>(pUp.y, pUp.x);
			depth = depth + depth4;
			Count++;

			vOridinal.push_back(Point3f(pUp.x, pUp.y, depth4));  //for test :�ڼ���ƽ��ֵ�Ĺ������Ƿ���ڻ���//
		}
	}

	if (pDown.x > 0 && pDown.x < m_cDepthWidth && pDown.y >0 && pDown.y < m_cDepthHeight) {
		if (!isBorderOrOut(cv16Img, srcContours, pDown, fBoundDepth)) {
			depth5 = cv16Img.at<USHORT>(pDown.y, pDown.x);
			depth = depth + depth5;
			Count++;

			vOridinal.push_back(Point3f(pDown.x, pDown.y, depth5));  //for test :�ڼ���ƽ��ֵ�Ĺ������Ƿ���ڻ���//
		}
	}

	//��ƽ��ֵ��8���������Count����//   


		if (pLeftUP.x > 0 && pLeftUP.x < m_cDepthWidth && pLeftUP.y >0 && pLeftUP.y < m_cDepthHeight) {
			if (!isBorderOrOut(cv16Img, srcContours, pLeftUP, fBoundDepth)) {
				depth6 = cv16Img.at<USHORT>(pLeftUP.y, pLeftUP.x);
				depth = depth + depth6;
				Count++;
				vOridinal.push_back(Point3f(pLeftUP.x, pLeftUP.y, depth6));  //for test :�ڼ���ƽ��ֵ�Ĺ������Ƿ���ڻ���//
			}
		}

		if (pLeftDown.x > 0 && pLeftDown.x < m_cDepthWidth && pLeftDown.y >0 && pLeftDown.y < m_cDepthHeight) {
			if (!isBorderOrOut(cv16Img, srcContours, pLeftDown, fBoundDepth)) {
				depth7 = cv16Img.at<USHORT>(pLeftDown.y, pLeftDown.x);
				depth = depth + depth7;
				Count++;
				vOridinal.push_back(Point3f(pLeftDown.x, pLeftDown.y, depth7)); //for test :�ڼ���ƽ��ֵ�Ĺ������Ƿ���ڻ���//
			}
		}

		if (pRightUp.x > 0 && pRightUp.x < m_cDepthWidth && pRightUp.y >0 && pRightUp.y < m_cDepthHeight) {
			if (!isBorderOrOut(cv16Img, srcContours, pRightUp, fBoundDepth)) {
				depth8 = cv16Img.at<USHORT>(pRightUp.y, pRightUp.x);
				depth = depth + depth8;
				Count++;

				vOridinal.push_back(Point3f(pRightUp.x, pRightUp.y, depth8));  //for test :�ڼ���ƽ��ֵ�Ĺ������Ƿ���ڻ���//
			}
		}

		if (pRightDown.x > 0 && pRightDown.x < m_cDepthWidth && pRightDown.y >0 && pRightDown.y < m_cDepthHeight) {
			if (!isBorderOrOut(cv16Img, srcContours, pRightDown, fBoundDepth)) {
				depth9 = cv16Img.at<USHORT>(pRightDown.y, pRightDown.x);
				depth = depth + depth9;
				Count++;
				vOridinal.push_back(Point3f(pRightDown.x, pRightDown.y, depth9));  //for test :�ڼ���ƽ��ֵ�Ĺ������Ƿ���ڻ���//
			}
		}

	//��ƽ��ֵ��8���������Count����//
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

				//����������жϣ�1������ǰ������ʱ任 2����1�Ļ����ϲ���ʱ�����þ�ֵ�ж�//
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

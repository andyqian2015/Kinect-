#include "stdafx.h"

#include "CCalib.h"


STDMETHODIMP CCalib::NonDelegatingQueryInterface(REFIID riid, void ** ppv)
{
	if (riid == __uuidof(GestureEng::ICalib)){
		return DLPGetInterface((GestureEng::ICalib*)this, ppv);
	}

	return __super::NonDelegatingQueryInterface(riid, ppv);
}


void CCalib::SetScreenPos(const std::vector<nvmath::Vec3f> & vRegZonePixel,const std::vector<nvmath::Vec3f> & vPixelPos, std::vector<nvmath::Vec3f> & vKinectPos)
{

}

//获取彩色像素的标定区域//
void CCalib::GetRegZone(std::vector<nvmath::Vec2f> & vPixelPos)
{
	//1:通过图像识别得到彩色图像的轮廓//
	
	int MinPixeIndexX = -1;
	int MaxPixelIndexX = -1;
	int MinPixelIndexY = -1;
	int MaxPixelIndexY = -1;
	GetMinX(m_CalibZoneCounter, MinPixeIndexX);
	GetMaxX(m_CalibZoneCounter, MaxPixelIndexX);
	GetMinY(m_CalibZoneCounter, MinPixelIndexY);
	GetMaxY(m_CalibZoneCounter, MaxPixelIndexY);
	
	//2:获取彩色图像边界轮廓//
	//先考虑正放的情形//
	nvmath::Vec2f v1(0, 0), v2(0, 0), v3(0, 0), v4(0, 0);
	v1 = nvmath::Vec2f(m_CalibZoneCounter[MinPixeIndexX].x, m_CalibZoneCounter[MinPixeIndexX].y);
	v4 = nvmath::Vec2f(m_CalibZoneCounter[MinPixeIndexX].x, m_CalibZoneCounter[MinPixeIndexX].y);
	if (v1[1] < v2[1])
		v4 = nvmath::Vec2f(m_CalibZoneCounter[MinPixelIndexY].x, m_CalibZoneCounter[MinPixelIndexY].y);
	else
		v3 = nvmath::Vec2f(m_CalibZoneCounter[MinPixelIndexY].x, m_CalibZoneCounter[MinPixelIndexY].y);

	vPixelPos.push_back(v1);
	vPixelPos.push_back(v2);
	vPixelPos.push_back(v3);
 	vPixelPos.push_back(v4);
}

bool CCalib::Precalc16_fast(cv::Mat & cv16img,int ndepthWidth, int ndepthHeight)
{
	for (int i = 0; i < ndepthHeight; i++) {
		for (int j = 0; j < ndepthWidth; j++) {
			V3f pImg;
			pImg.x = j;
			pImg.y = i;
			pImg.z = cv16img.ptr<USHORT>(i)[j];

			//在实际使用中，当Kinect置于屏幕上方时，确定区域的范围是用Kinect的y值来确定的，因此在限定范围的时候，不能用depth来限定//
			//不能利用像素值对图像识别区域边界进行限定，当屏幕离Kinect较近时，可识别的y的区域较小，物体稍微离屏远一点，就会出了屏幕识别区域//
			bool flag = false;
			if (pImg.z> m_iMinLimitz_1 && pImg.z < m_iMaxLimitz_1 && j > m_iMinLimitx_1 && j < m_iMaxLimitx_1) {
				V3f pImgKinectPos(0, 0, 0), projectP(0, 0, 0);
				m_PIMathBase->getKinectPos(pImg, pImgKinectPos, ndepthWidth, ndepthHeight);
				m_PIMathBase->getProjectPos(m_CalibPlane,pImgKinectPos, projectP);
				Mult(projectP, m_vProToCalibPlanePos);
				if (m_vProToCalibPlanePos.x > m_fMinIdentX  && m_vProToCalibPlanePos.x < m_fMaxIdentX && m_vProToCalibPlanePos.y > m_fMinIdentY && m_vProToCalibPlanePos.y < m_fMaxIdentY)
				{
					flag = true;
				}

				if (flag) {
					float fdist = (projectP - pImgKinectPos).length();
					if (fdist < 500 || fdist > 50)  {
						flag = false;
					}
				}
			}

			if (!flag) {
				cv16img.ptr<USHORT>(i)[j] = 0;
			}
		}
	}

	for (int i = 0; i < ndepthHeight; i++) {
		for (int j = 0; j < ndepthWidth; j++) {
			m_cv8Img.ptr<uchar>(i)[j] = (uchar)(cv16img.at<USHORT>(i, j) * 255.0 / m_iMaxLimitz);
		}
	}

	return  true;
}

BOOL CCalib::SeekHands(cv::Mat& cv16img, V3f & vHandCenterPos,const int &imgwidth, const int & imgheight)
{
	//利用二值图获取轮廓//
	g_pGrayImage = &IplImage(m_cv8Img);
	//将灰度图转化成二值图//
	g_pBinaryImage = cvCreateImage(cvGetSize(g_pGrayImage), IPL_DEPTH_8U, 1);
	on_trackbar(0, m_contours, imgwidth, imgheight);

#define HAND_LIKELY_AREA 250
	cv::Point2f cen(0,0); float radius = 0;

	for (unsigned i = 0; i < m_contours.size(); i++)
	{
		cv::Mat contour_mat = cv::Mat(m_contours[i]);
		float area = contourArea(contour_mat);
		if (contourArea(contour_mat) > HAND_LIKELY_AREA)
		{
			minEnclosingCircle(m_contours[i], cen, radius);
		}
	}

	if (cen.x == 0 || cen.y == 0)
		return false;

	float fdepth = cv16img.at<USHORT>(cen.y,cen.x);
	if (fdepth == 0)
		return false;

	vHandCenterPos = V3f(cen.x, cen.y, fdepth);
	return true;
}

void CCalib::GetMinX(const std::vector<cv::Point> & counters, int & MinPixelXIndex)
{
	int nMinPixelX = 10000;
	for (int i = 0; i < counters.size(); i++) {
		if (nMinPixelX > counters[i].x) {
			nMinPixelX = counters[i].x;
			MinPixelXIndex = i;
		}
	}
}

void CCalib::GetMaxX(const std::vector<cv::Point> & counters, int & MaxPixelXIndex)
{
	int nMaxPixelX = -1;
	for (int i = 0; i < counters.size(); i++) {
		if (nMaxPixelX  < counters[i].x) {
			nMaxPixelX = counters[i].x;
			MaxPixelXIndex = i;
		}
	}
}

void CCalib::GetMinY(const std::vector<cv::Point> & counters, int & MinPixelYIndex)
{
	int nMinPixelY = 10000;
	for (int i = 0; i < counters.size(); i++) {
		if (nMinPixelY > counters[i].y) {
			nMinPixelY = counters[i].y;
			MinPixelYIndex = i;
		}
	}
}

void CCalib::GetMaxY(const std::vector<cv::Point> & counters, int & MaxPixelYIndeY)
{
	int nMaxPixelY = -1;
	for (int i = 0; i < counters.size(); i++) {
		if (nMaxPixelY < counters[i].x) {
			nMaxPixelY = counters[i].x;
			MaxPixelYIndeY = i;
		}
	}
}


//OutPut:手势在屏幕上投影的深度像素以及深度值//
void CCalib::GetProPixelPoints(cv::Mat& cv16img, V3f & vScreemPixelPos, int width, int height, V3f & PixelPos)
{
	V3f vHandCenterPos(0, 0, 0);
	SeekHands(cv16img, vHandCenterPos, width, height);

	if (!m_PIMathBase)
		return;

	V3f vKinectPos(0, 0, 0);
	m_PIMathBase->getKinectPos(vHandCenterPos, vKinectPos, width, height);

	//test for use//
	m_CalibPlane = Plane3f(m_vMarkPoint[0], m_vMarkPoint[1], m_vMarkPoint[2]);
	V3f proPos(0, 0, 0);

	//2: pro to screem  kinect pos//
	m_PIMathBase->getProjectPos(m_CalibPlane, vKinectPos, proPos);
	//3:  screen pixel points//
	m_PIMathBase->getPixelPos(proPos, PixelPos, width, height);
}

void  CCalib::GetAllScreenKinectPos(std::vector<V3f> & ScreenKinectPos)
{
	int nCompensateIndex = -1;
	for (int i = 0; i < ScreenKinectPos.size(); i++) {
		if (ScreenKinectPos[i] == V3f(0, 0, 0)) {
			nCompensateIndex = i;
			break;
		}
	}
	if (nCompensateIndex == 2)
		GetCompensatePixelPoints(ScreenKinectPos[0], ScreenKinectPos[1], ScreenKinectPos[3], ScreenKinectPos[2]);

	else if (nCompensateIndex == 3)
		GetCompensatePixelPoints(ScreenKinectPos[1], ScreenKinectPos[0], ScreenKinectPos[2], ScreenKinectPos[3]);

	else
		BOOST_ASSERT(!"the ScreenKinectPos miss !");

	m_vMarkPoint = ScreenKinectPos;
}


void CCalib::GetCompensatePixelPoints(const V3f & v1, const V3f & v2, const V3f & v3, V3f & v4)
{
	v4 = (v2 + v3 - v1);
}

CCalib::CCalib()
{
	m_PIMathBase = createPIMathBase();
}

//test for use//
void CCalib::GetAllCounter(const int &colorwidth, const int & colorheight, const void* pColorBuffer)
{
	GetColorMat(colorwidth, colorheight, pColorBuffer);
	GetCounter(colorwidth, colorheight);
}
//test for use//



//the most import step : to get the bound point : use the color image//
//1: 将RGB的图像像素转换成二值图，具体的为：(R+G+B)/(3*255)//
void CCalib::GetColorMat(const int &colorwidth, const int & colorheight, const void* pColorBuffer)
{

	RGBQUAD pColor;
	int red = 0, blue = 0, green = 0;
	m_cv8Img = cv::Mat(colorheight, colorwidth, CV_8UC1);
	for (int i = 0; i < colorheight; i++) {
		for (int j = 0; j < colorwidth; j++) {
			pColor = *((RGBQUAD*)pColorBuffer + i *  colorwidth + j);
			red = pColor.rgbRed;
			green = pColor.rgbGreen;
			blue = pColor.rgbBlue;


//			if ((j > 890 && j < 1690) && (i >610 && i < 970)) {
//				if (red > 150 && green > 150 && blue > 150)
					m_cv8Img.ptr<uchar>(i)[j] = (uchar)((int((red + green + blue) / 3)));

//				else
// 					m_cv8Img.ptr<uchar>(i)[j] = 0;
//			}

// 			else {
//				m_cv8Img.ptr<uchar>(i)[j] = (uchar)0;
//			}
         }
	}

}


void CCalib::GetCounter(const int & colorwidth, const int & colorheight)
{
// 	//利用二值图获取轮廓//
 	g_pGrayImage = &IplImage(m_cv8Img);
// 
// 	//将灰度图转化成二值图//
 	g_pBinaryImage = cvCreateImage(cvGetSize(g_pGrayImage), IPL_DEPTH_8U, 1);
 	on_trackbar(150, m_contours, colorwidth, colorheight);


//	cv::findContours(m_cv8Img, m_contours, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE);

//	vector<Vec4i> hierarchy;

//	cv::findContours(m_cv8Img, m_contours, hierarchy, CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE);

	//test//

	//(j > 890 && j < 1690) && (i >610 && i < 970)

	std::vector<int>  m_ValidCounter(0);
	for (int i = 0; i < m_contours.size(); i++) {

		int k = 0;
		for (; k < m_contours[i].size(); k++) 
		{
			if (m_contours[i][k].x < 890 || m_contours[i][k].x < 1690 || m_contours[i][k].y < 610 || m_contours[i][k].y > 970)
				break;
		}

		if (k == m_contours[i].size())
			m_ValidCounter.push_back(i);
	}
	

	float fMaxArea  = -1;
	int   nMaxindex = -1;
	for (int i = 0; i < m_contours.size(); i++) {
		cv::Mat contour_mat = cv::Mat(m_contours[i]);
		float area = contourArea(contour_mat);
		if (area > fMaxArea)
		{
			fMaxArea  = area;
			nMaxindex = i;
		}
	}
	if (nMaxindex >= 0) {
		m_CalibZoneCounter = m_contours[nMaxindex];
	}
	
	//test show boundary counters//
	cv::Mat MyMat = cv::Mat::zeros(colorheight, colorwidth, CV_8UC1);

	for (int i = 0; i < m_CalibZoneCounter.size(); i++) {
		MyMat.at<uchar>(m_CalibZoneCounter[i].y, m_CalibZoneCounter[i].x) = 255;
	}

//	imshow("savedGrayMat",MyMat);
	std::vector<nvmath::Vec2f>vPixelPos;
	GetRegZone(vPixelPos);
	int testm = 10;


}


void CCalib::on_trackbar(int pos, std::vector<vector<Point>> & contours, const int &imgwidth ,const int & imgheight)
{
	// 转为二值图
	cvThreshold(g_pGrayImage, g_pBinaryImage, pos, 255, CV_THRESH_BINARY); 
	// 显示二值图

	cv::Mat MbinaryMat(g_pBinaryImage, 0);

//	findContours(MbinaryMat, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);

	vector<Vec4i> hierarchy;
	findContours(MbinaryMat, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);

	cv::Mat MyMat = cv::Mat::zeros(imgheight, imgwidth, CV_8UC1);

	std::vector<vector<Point>>  copycontours(0);


	for (int i = 0; i < contours.size(); i++) {
		cv::Mat contour_mat = cv::Mat(contours[i]);
		if (contourArea(contour_mat) > 5000) {
			for (int k = 0; k < contours[i].size(); k++) {
				MyMat.at<uchar>(contours[i][k].y, contours[i][k].x) = 255;
			}
		}
	}
	drawContours(MyMat, contours, 0, Scalar(255), CV_FILLED);
	imshow("savedGrayMat", MyMat);

	//tets//


	float fMaxArea = -1;
	int   fMaxIndex = -1;
	for (int i = 0; i < contours.size(); i++) {
		cv::Mat contour_mat = cv::Mat(contours[i]);
		float m = contourArea(contour_mat);
		if (fMaxArea < m) {
			fMaxArea = m;
			fMaxIndex = i;
		}
			
	}

	int testm = 10;

}





//标定平面的限制//
void CCalib::GetZoneRestrict()
{
	//获取识别区域的深度以及对应的像素点//
	m_iMinLimitx_1 = min(min(m_vpixelKinectPos[0][0], m_vpixelKinectPos[1][0]), min(m_vpixelKinectPos[2][0], m_vpixelKinectPos[3][0]));
	m_iMaxLimitx_1 = max(max(m_vpixelKinectPos[0][0], m_vpixelKinectPos[1][0]), max(m_vpixelKinectPos[2][0], m_vpixelKinectPos[3][0]));
	m_iMaxLimity_1 = max(max(m_vpixelKinectPos[0][1], m_vpixelKinectPos[1][1]), max(m_vpixelKinectPos[2][1], m_vpixelKinectPos[3][1]));
	m_iMinLimity_1 = min(min(m_vpixelKinectPos[0][1], m_vpixelKinectPos[1][1]), min(m_vpixelKinectPos[2][1], m_vpixelKinectPos[3][1]));
	m_iMaxLimitz_1 = max(max(m_vpixelKinectPos[0][2], m_vpixelKinectPos[1][2]), max(m_vpixelKinectPos[2][2], m_vpixelKinectPos[3][2]));
	m_iMinLimitz_1 = min(min(m_vpixelKinectPos[0][2], m_vpixelKinectPos[1][2]), min(m_vpixelKinectPos[2][2], m_vpixelKinectPos[3][2]));


	//利用Kinect坐标系下的标定平面坐标对识别区域进行限制//
	m_iMaxLimitx = max(min(m_vKinectPos[0][0], m_vKinectPos[1][0]), min(m_vKinectPos[2][0], m_vKinectPos[3][0]));
	m_iMinLimitx = min(max(m_vKinectPos[0][0], m_vKinectPos[1][0]), max(m_vKinectPos[2][0], m_vKinectPos[3][0]));

	m_iMaxLimitz = max(max(m_vKinectPos[0][2], m_vKinectPos[1][2]), max(m_vKinectPos[2][2], m_vKinectPos[3][2]));
	m_iMinLimitz = min(min(m_vKinectPos[0][2], m_vKinectPos[1][2]), min(m_vKinectPos[2][2], m_vKinectPos[3][2]));
}


////
void SetCalibPlane()
{

}


void CCalib::Mult(const V3f &vec, V3f &v)
{
	V4f v1(vec.x, vec.y, vec.z, 1);
	v.x = m_invTransFromMat[0][0] * v1.x + m_invTransFromMat[0][1] * v1.y + m_invTransFromMat[0][2] * v1.z + m_invTransFromMat[0][3] * v1.w;
	v.y = m_invTransFromMat[1][0] * v1.x + m_invTransFromMat[1][1] * v1.y + m_invTransFromMat[1][2] * v1.z + m_invTransFromMat[1][3] * v1.w;
	v.z = m_invTransFromMat[2][0] * v1.x + m_invTransFromMat[2][1] * v1.y + m_invTransFromMat[2][2] * v1.z + m_invTransFromMat[2][3] * v1.w;
}


bool CCalib::SetKinectCoordinateSystem()
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


void CCalib::GetInentZone()
{
	V3f v1(0, 0, 0), v2(0, 0, 0), v3(0, 0, 0), v4(0, 0, 0);
	Mult(m_vKinectPos[0], v1);
	Mult(m_vKinectPos[1], v2);
	Mult(m_vKinectPos[2], v3);
	Mult(m_vKinectPos[3], v4);
	m_fMaxIdentX = max(max(v1.x, v2.x), max(v3.x, v4.x));
	m_fMinIdentX = min(min(v1.x, v2.x), min(v3.x, v4.x));
	m_fMaxIdentY = max(max(v1.y, v2.y), max(v3.y, v4.y));
	m_fMinIdentY = min(min(v1.y, v2.y), min(v3.y, v4.y));
}

void CCalib::SetScreenPos_1(const std::vector<nvmath::Vec3f> &vpixelKinectPos, const std::vector<nvmath::Vec3f>&vKinectPos)
{
	m_vpixelKinectPos.clear();
	m_vKinectPos.clear();
	for (int i = 0; i < vpixelKinectPos.size(); i++) {
		m_vpixelKinectPos[i] = V3f(vpixelKinectPos[i][0], vpixelKinectPos[i][1], vpixelKinectPos[i][2]);
		m_vKinectPos[i] = V3f(vKinectPos[i][0], vKinectPos[i][1], vKinectPos[i][2]);
	}

	

}

CCalib::~CCalib()
{

}



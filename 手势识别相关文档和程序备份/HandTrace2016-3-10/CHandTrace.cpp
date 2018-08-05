#include "CHandTrace.h"


IplImage *g_pGrayImage = NULL;
IplImage *g_pBinaryImage = NULL;

int startTime1 = 0;
int endTime1 = 0;
int endTime2 = 0;



namespace HandTrace {
	PIHandTrace createPIHandTrace()
	{
		PIHandTrace  p(new CHandTrace);
		return p;
	}
}


void CHandTrace::SetVidient()
{
	this->videntPixelx.clear();
	this->videntPixely.clear();
}

void  CHandTrace::InitData(const std::vector<V3f> & vmarkscreenpos, const std::vector<V3f> & vmarkKinectPos, int KinectInstallPattern,float h, float h1, float Radius) {

	m_fHandThreshold = 20;  //�����ж϶�����Ƶ���ֵ���������С�ڸ���ֵ������Ϊ��һ������ ɾ�������������//


	useTime1 = useTime2 = 0;  //test//
	this->KinectInstallPattern = KinectInstallPattern; //����Kinect�İ�װ��ʽ//
	m_CritMarkPoint = cv::Point2f(0, 0);

	//Pictute Mat init/
 	cv8Img = cv::Mat(cDepthHeight, cDepthWidth, CV_8UC1);
	cv16Img = cv::Mat(cDepthHeight, cDepthWidth, CV_16UC1);  //for test

	for (int i = 0; i < cDepthHeight; i++) {
		for (int j = 0; j < cDepthWidth; j++) {
			cv8Img.at<uchar>(i, j) = 0;
			cv16Img.at<USHORT>(i, j) = 0;
		}
	}

	fSummitPoint = cv::Point(0, 0); //�����Ƽ�˵㸳��ֵ//

	//�������˶����ٶȳ�ʼ��//
	fHandPosThreshold = 200;

	depthrevisethreshold = 80;

	fOutThreshold = 10;

	iPointthreshold = 30;

	fActorOnOneSidethreshold = 20;  //�������Ƿ���һ�����ֵ//

	bIsInOrOutFlag = true;

	identPixelx = identPixely = 0;
	identPixelx1 = identPixely1 = 0;
	identPixelx2 = identPixely2 = 0;
	identPixelx3 = identPixely3 = 0;


	if (min(vmarkKinectPos[0].x, vmarkKinectPos[1].x) > min(vmarkKinectPos[2].x, vmarkKinectPos[3].x)) {
		minPixelx = max(vmarkKinectPos[2].x, vmarkKinectPos[3].x);
	}

	else  {
		minPixelx = max(vmarkKinectPos[0].x, vmarkKinectPos[1].x);
	}
	//	minPixelx = min(min(vmarkKinectPos[0].x, vmarkKinectPos[1].x), min(vmarkKinectPos[2].x, vmarkKinectPos[3].x));

	if (min(vmarkKinectPos[0].y, vmarkKinectPos[3].y) > min(vmarkKinectPos[2].y, vmarkKinectPos[1].y)) {
		minPixely = max(vmarkKinectPos[1].y, vmarkKinectPos[2].y);
	}

	else {
		minPixely = max(vmarkKinectPos[0].y, vmarkKinectPos[3].y);
	}

	if (max(vmarkKinectPos[0].x, vmarkKinectPos[1].x) > max(vmarkKinectPos[2].x, vmarkKinectPos[3].x)) {
		maxPixelx = min(vmarkKinectPos[0].x, vmarkKinectPos[1].x);
	}

	else {
		maxPixelx = min(vmarkKinectPos[2].x, vmarkKinectPos[3].x);
	}

	if (max(vmarkKinectPos[0].y, vmarkKinectPos[3].y) > max(vmarkKinectPos[1].y, vmarkKinectPos[2].y)) {
		maxPixely = min(vmarkKinectPos[0].y, vmarkKinectPos[3].y);
	}

	else {
		maxPixely = min(vmarkKinectPos[1].y, vmarkKinectPos[2].y);
	}




	this->h = h;
	this->h1 = h1;
	this->Radius = Radius;

	this->sCreenPos = vmarkscreenpos;

	this->vpixelKinectPos = vmarkKinectPos;
	iMinLimitx_1 = min(min(vmarkKinectPos[0].x, vmarkKinectPos[1].x), min(vmarkKinectPos[2].x, vmarkKinectPos[3].x));
	iMaxLimitx_1 = max(max(vmarkKinectPos[0].x, vmarkKinectPos[1].x), max(vmarkKinectPos[2].x, vmarkKinectPos[3].x));
	iMaxLimity_1 = max(max(vmarkKinectPos[0].y, vmarkKinectPos[1].y), max(vmarkKinectPos[2].y, vmarkKinectPos[3].y));
	iMinLimity_1 = min(min(vmarkKinectPos[0].y, vmarkKinectPos[1].y), min(vmarkKinectPos[2].y, vmarkKinectPos[3].y));
	iMaxLimitz_1 = max(max(vmarkKinectPos[0].z, vmarkKinectPos[1].z), max(vmarkKinectPos[2].z, vmarkKinectPos[3].z));
	iMinLimitz_1 = min(min(vmarkKinectPos[0].z, vmarkKinectPos[1].z), min(vmarkKinectPos[2].z, vmarkKinectPos[3].z));


	iMaxLimitx_2 = max(min(vmarkKinectPos[0].x, vmarkKinectPos[1].x), min(vmarkKinectPos[2].x, vmarkKinectPos[3].x));
	iMinLimitx_2 = min(max(vmarkKinectPos[0].x, vmarkKinectPos[1].x), max(vmarkKinectPos[2].x, vmarkKinectPos[3].x));
	iMaxLimity_2 = max(min(vmarkKinectPos[0].x, vmarkKinectPos[1].x), min(vmarkKinectPos[2].x, vmarkKinectPos[3].x));
	iMinLimity_2 = min(max(vmarkKinectPos[0].x, vmarkKinectPos[1].x), max(vmarkKinectPos[2].x, vmarkKinectPos[3].x));

	std::vector<V3f> vKinectPos = vmarkKinectPos;

	for (int i = 0; i < vKinectPos.size(); i++) {
		V3f p = vKinectPos[i];
		vKinectPos[i] = getKinectPos(p);
	}


	//test:�ж�Kinect������ȶ��ԣ�ת��ΪKinect�����Ժ��Ƿ��Ǿ���//
	V3f vTest1 = vKinectPos[1] - vKinectPos[0];
	V3f vTest2 = vKinectPos[3] - vKinectPos[0];
	float fTestAngle1 = (vTest1.normalize()).dot(vTest2.normalize());
	V3f vTest3 = vKinectPos[0] - vKinectPos[1];
	V3f vTest4 = vKinectPos[2] - vKinectPos[1];
	float fTestAngle2 = (vTest3.normalize()).dot(vTest4.normalize());
	V3f vTest5 = vKinectPos[3] - vKinectPos[2];
	V3f vTest6 = vKinectPos[1] - vKinectPos[2];
	float fTestAngle3 = (vTest5.normalize()).dot(vTest6.normalize());
	V3f vTest7 = vKinectPos[2] - vKinectPos[3];
	V3f vTest8 = vKinectPos[0] - vKinectPos[3];
	float fTestAngle4 = (vTest7.normalize()).dot(vTest8.normalize());


	this->vKinectPos = vKinectPos;
	SetKinectCoordinateSystem();
	GetInentZone();

	//����Linect����ϵ�µ������ʶ�������������//
	iMaxLimitx = max(min(vKinectPos[0].x, vKinectPos[1].x), min(vKinectPos[2].x, vKinectPos[3].x));
	iMinLimitx = min(max(vKinectPos[0].x, vKinectPos[1].x), max(vKinectPos[2].x, vKinectPos[3].x));

	iMaxLimitz = max(max(vKinectPos[0].z, vKinectPos[1].z), max(vKinectPos[2].z, vKinectPos[3].z));
	iMinLimitz = min(min(vKinectPos[0].z, vKinectPos[1].z), min(vKinectPos[2].z, vKinectPos[3].z));

	V3f v1, v2, v3, v4, v_1, v_2, v_3, v_4;
	v1 = vKinectPos[0];
	v2 = vKinectPos[1];
	v3 = vKinectPos[2];
	v4 = vKinectPos[3];

	std::vector<float> vLength;
	float length1 = (v2 - v1).length();
	float length2 = (v3 - v2).length();
	float length3 = (v4 - v3).length();
	float length4 = (v1 - v4).length();
	float del1 = abs(length1 - 650);
	float del2 = abs(length2 - 920);
	float del3 = abs(length3 - 650);
	float del4 = abs(length4 - 920);   //�����߶�270mm//  //��������Ϊ400mm,���ֱ�Ϊ��14/13/8/5mm//

	Plane3f screenPlane(v1, v2, v3);
	pScreenPlane = screenPlane;


	v_1 = v1;
	v_2 = v2;
	v_3 = v3;
	v_4 = v4;
	Plane3f pDistScreen = GetPlane(v_1, v_2, v_3, v_4);

	vdisMarkPoint.push_back(v_1);
	vdisMarkPoint.push_back(v_2);
	vdisMarkPoint.push_back(v_3);
	vdisMarkPoint.push_back(v_4);

	this->iMaxDepth = getMaxDepth();
	this->iMinDepth = getMinDepth();

	this->pdisScreenPlane = pDistScreen;


	//test:������ƽ��ͶӰ�ĵ����Kinectx��ֵ������ͬ//
	V3f vTestPixel(399, 236, 1298);
	V3f KinectPos = getKinectPos(vTestPixel);
	V3f vProjectPoint = getProjectPos(KinectPos);
	V3f vScreenPos = convertTo(vProjectPoint);

}

std::vector<cv::Point2f> CHandTrace::GetCounterSummitPoint()
{
	std::vector<cv::Point2f> ContourSummitPoint;
	ContourSummitPoint.resize(videntPixelx.size());
	for (int i = 0; i < ContourSummitPoint.size(); i++) {
		ContourSummitPoint[i] = cv::Point2f (videntPixelx[i], videntPixely[i]);
	}
	return  ContourSummitPoint;
}


//test:���ͼ��������ص�����//

std::vector<cv::Point3f> CHandTrace::GetCounterSummitPoint3()
{
	std::vector<cv::Point3f> ContourSummitPoint;
	ContourSummitPoint.resize(videntPixelx.size());
	for (int i = 0; i < ContourSummitPoint.size(); i++) {
		ContourSummitPoint[i] = cv::Point3f(videntPixelx[i], videntPixely[i], cv16Img.at<USHORT>(videntPixely[i], videntPixelx[i]));
	}
	return  ContourSummitPoint;
}


UINT  CHandTrace::getMaxDepth() {
	UINT maxDepth = max(max(vKinectPos[0].z, vKinectPos[1].z), max(vKinectPos[2].z, vKinectPos[3].z));
	return maxDepth;
}

UINT  CHandTrace::getMinDepth() {
	UINT minDepth = min(min(vdisMarkPoint[0].z, vdisMarkPoint[1].z), min(vdisMarkPoint[2].z, vdisMarkPoint[3].z));
	return minDepth;

}

std::vector<sAction>    CHandTrace::GetHandGesture(cv::Mat &cv16img)
{

	startTime1 = clock();
	Precalc16_fast(cv16img);
	endTime1 = clock();
	useTime1 = endTime1 - startTime1;
	SeekHands(cv16img);
	endTime2 = clock();
	useTime2 = endTime2 - endTime1;
	return m_hands;
}

int CHandTrace::SeekHands(cv::Mat& cv16img)
{


	//	return 0;
	//����ǰһ֡��up�������������Ϊ��ǰ֡�Ĳο�//
	std::vector<sAction> vCopyPreHands;
	for (int i = 0; i < m_hands.size(); i++) {
		if (m_hands[i].st != Up) {
			vCopyPreHands.push_back(m_hands[i]);
		}
	}
	m_hands.clear();

	//test//
	//���ö�ֵͼ��ȡ����//
	g_pGrayImage = &IplImage(cv8Img);

	//���Ҷ�ͼת���ɶ�ֵͼ//
	g_pBinaryImage = cvCreateImage(cvGetSize(g_pGrayImage), IPL_DEPTH_8U, 1);
	std::vector<vector<Point>> contours;
	on_trackbar(0, contours);
	//for test//


	//	std::vector< std::vector<Point> > contours;
	//	cv::findContours(cv8img, contours, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE);  //call findContours to get contours//
#define HAND_LIKELY_AREA 40

	for (unsigned i = 0; i < contours.size(); i++)
	{
		//ֻ���ڼ�⵽����ʱ�Ż�ȥ�����Ķ���Σ�͹���������ݼ�
		cv::Mat contour_mat = cv::Mat(contours[i]);

		float area = contourArea(contour_mat);

		if (contourArea(contour_mat) > HAND_LIKELY_AREA)
		{   //�Ƚ��п������ֵ�����

			//���Ա궨�����ⲿ�ĵ�ΪʲôҲ��ʶ��ĵ�//
			cv::Point3f imgpt;

			cv::Point2f cen; float radius = 0;
			minEnclosingCircle(contours[i], cen, radius);

			//test//
			//����������С���ԲԲ�ĺ������ı߽���ֵ��ȷ�����Ƶı߽��//

			std::vector<cv::Point> MyContours;

			float dist = 0, dist1 = 0;
			cv::Point pMyPoint;

			std::vector<int> oridinal;

			std::vector<int> vdelneighbourPoint;  //�޳������Ѿ����ڵ���MyCotours�е������еĵ�//

			//�����п��ܳ��ֵģ��ҵ��Ľϴ�ֵ�������Ѿ��ҵ��ı߽����������������ʹ�ú���ķ���Ҫ�ı߽��������������Ұ�У���ȡ�ķ���Ϊ���趨�����ֵ���������Ŀ���������ֵ������Ϊ�õ�����СԲ���ڲ��㣬����ȡ��//

			while (MyContours.size() < 5 && (oridinal.size()< iPointthreshold)) {
				bool bAddVectorElement = true;
				int imaxindex = 0;
				for (int j = 0; j < contours[i].size(); j++) {
					int k = 0;
					bool flag = true;


					//�޳���ȡ���Ľϴ�dist//
					while (k < oridinal.size()) {
						if (contours[i][oridinal[k]] == contours[i][j]) {
							flag = false;
							break;
						}
						k++;
					}

					if (flag) {
						cv::Point p = contours[i][j];
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
				for (int i = 0; i < MyContours.size(); i++) {
					cv::Point pMyContours = MyContours[i];
					if ((abs(pMyPoint.x - pMyContours.x) <= 10) && (abs(pMyPoint.y - pMyContours.y) <= 10)) {
						bAddVectorElement = false;
						break;
					}
				}

				//��һ���ж���䣬��������������ıȽϽ���ֻѡ������һ����Ϊĩ�����ص�//
				if (bAddVectorElement) {
					MyContours.push_back(pMyPoint);
				}
			}


			//test//
			//���Զ���������ȵ���//

// 			std::vector<cv::Point> vCopyPreContours;
// 			vCopyPreContours = MyContours;
// 
// 			//test1:����ǰ��Ա�//
// 			std::vector<float>vPreDepth;
// 			float depth = 0;
// 			for (int i = 0; i < MyContours.size(); i++) {
// 				depth = cv16img.at<USHORT>(MyContours[i].y, MyContours[i].x);
// 				vPreDepth.push_back(depth);
// 			}
// 			//test:����ǰ��Ա�//
// 
// 			depthRevise(cv16img, contours[i], MyContours);
// 
// 
// 			//test:�������ֵ�Ƿ��и���//
// 			std::vector<float>vAfterDepth;
// 
// 			for (int i = 0; i < MyContours.size(); i++) {
// 				depth = cv16img.at<USHORT>(MyContours[i].y, MyContours[i].x);
// 				vAfterDepth.push_back(depth);
// 			}

			//test:�������ֵ�Ƿ��и���//

			//��˵�仯�ϴ�ĵ���С�ķ����ĵ�һ������ָ��˵�//  //��ƽ��ֵ�͸�����ֵ�����Сʱ��˵��������ֵ�ǿɿ��ģ�����ʹ�ã���ʱ�򿴼��ֵ��ƽ��ֵ�Ĺ�ϵ������Ǳ�С�ģ������С�Ľϴ�һ��Ϊ��˵�//

			//�õ���˵�����һ�ַ���������Ҫ�����Ƽ�����Ե��֮���������Ȳ���Բ��ʺ�����������һ�ߵ����//
			//test:��������ȷ����˵ķ�������Ч��//


			//test//
			//��������������Ļһ�������//

			//������վ����Ļ��һ��//   //һ�������������ߣ������������³�ȥ����˲���Ҫ��������y�����жϣ���ö�ͳһΪ����ֵ���ж�//
			//��Ϊ��һ��ʱ�����Ƽ�˵�x���ص�����Զ��ñߣ������������ߣ���ѡ���Ӧ����߽�Զ��x���صĵ㣬������ұߣ���ѡ�����ұ߽�Զ��x���ص��Ӧ�ĵ�// 
			bool bOneSideflag = getSummitPoint(fActorOnOneSidethreshold, MyContours, fSummitPoint, cv16img);

			bool bInScreenFlag = true;

			//*������ȵ����㷨��ȡ�����Ƽ�˵�*//


			if (!bOneSideflag) {

				//test//
				//����K-Means������ȡ�����Ƽ�˵�//
				if ((MyContours.size() > 0)) {
					//test//
					//���ڻ�ȡ��ָ���λ��//
					fSummitPoint = getSummitPoint(MyContours);    //���ڻ�ȡ��˵�����//
					//���ڻ�ȡ��ָ���λ��//
				}
				//����K-Means������ȡ�����Ƽ�˵�//
				//test//
			}


			std::vector<cv::Point> vCopyPreContours(1);
			vCopyPreContours[0] = fSummitPoint;

			identPixelx = fSummitPoint.x;
			identPixely = fSummitPoint.y;


			//for test//
			videntPixelx.push_back(fSummitPoint.x);
			videntPixely.push_back(fSummitPoint.y);


			//ps:����״̬��ʧ�ĵ��״̬������ǰ���״̬���//

			//test:��������ȷ����˵ķ�������Ч��//
			//�õ���˵�����һ�ַ���������Ҫ�����Ƽ�����Ե��֮���������Ȳ���Բ��ʺ�����������һ�ߵ����//


			//���Զ���������ȵ���//
			//test//

			//*********************problem***************************//

			//Kinect����������ݻ�û�в⣬���ڵ������ǣ���ĳ������£���⵽���������ڱ仯�ģ���˴���ʶ��ı�Ե��׼ȷ�������Ҫ����һ��//

			//*********************problem***************************//



			//test :����ָ���ض��Ǳ߽��õ�ָ����λ��//

			//test :����ָ���ض��Ǳ߽��õ�ָ����λ��//

			identPixel = MyContours;

			imgpt.x = fSummitPoint.x;
			imgpt.y = fSummitPoint.y;
			imgpt.z = cv16img.at<USHORT>(fSummitPoint.y, fSummitPoint.x);




			float PixelDist = 0;
			float fMinPixelDist = 10000;
			int iMinPixelDistIndex = -1;
			V3f p;
			V3f PixelPos;
			for (int i = 0; i < vCopyPreHands.size(); i++) {

				p = V3f(vCopyPreHands[i].worldPos.x, vCopyPreHands[i].worldPos.y, vCopyPreHands[i].worldPos.z);
				PixelPos = getPixelPos(p);

				PixelDist = abs(fSummitPoint.x - PixelPos.x) + abs(fSummitPoint.y - PixelPos.y);
				if (PixelDist < fMinPixelDist) {
					fMinPixelDist = PixelDist;
					iMinPixelDistIndex = i;
				}

			}

			if (iMinPixelDistIndex >= 0) {
				p = V3f(vCopyPreHands[iMinPixelDistIndex].worldPos.x, vCopyPreHands[iMinPixelDistIndex].worldPos.y, vCopyPreHands[iMinPixelDistIndex].worldPos.z);
				PixelPos = getPixelPos(p);
				if (abs(fSummitPoint.x - PixelPos.x) < 5 && abs(fSummitPoint.y - PixelPos.y) < 5) {
					if (abs(fSummitPoint.x - PixelPos.x) + abs(fSummitPoint.y - PixelPos.y) < 8) {
						imgpt.z = vCopyPreHands[iMinPixelDistIndex].worldPos.z;
					}
				}
			}

			V3f vImgpt;
			vImgpt.x = imgpt.x;
			vImgpt.y = imgpt.y;
			vImgpt.z = imgpt.z;




			//test//
			//�����ж������Ƿ���ʶ��������//
			//��Ϊ�������򲢲������ǵ�ʶ����������ڴ˵ػ���Ҫ�жϸ����ĵ��Ƿ���ʶ�����򣬽����жϣ���������������õ���Ա�ʶ�𵽣����Ǹõ㲻��ʶ�������У������Ҫ�Ե��Ƿ���ʶ����������ж�//

			V3f KinectPos = getKinectPos(vImgpt);
			if (KinectPos.x == 0 && KinectPos.y == 0) {
				ATLTRACE("%d\n", 2);
			}
			if (vfKinectPos.size() >= 2) {
				ATLTRACE("%d\n", 2);
			}

			//��ȡ���Ƶ��Kinectpos,Ϊȷ�����Ƶ��ǰ��֡����//
			vfKinectPos.push_back(KinectPos);  //�ж���ֻ�֣�vfKinectPos��size���ж��//

			//�����ж������Ƿ���ʶ��������//
			//test//
		}
	}
	GetReviseHandPos();  //�޳������������//
	if (vfKinectPos.size() >= 2) {
		ATLTRACE("%d\n", 2);
	}
	std::fstream out3("D:\\HandCount.txt", std::ios::app);
	out3 << vfKinectPos.size() << std::endl;


	//�����Ƕ������������м��//

	//test//
	//����fSummitPoint������//


// 	std::fstream out2("D:\\fSummitPoint.txt", std::ios::app);
// 	out2 << fSummitPoint.x << " " << fSummitPoint.y << " " << cv16img.at<USHORT>(fSummitPoint.y, fSummitPoint.x) << std::endl;




	//************************** need to consider further *************************//
	//��ȡǰ��֡�����Ƶ�KinectPos//  //��һ����Ҫ���ǵ���m_hands.size() ��һ����//        //����vPos��ô��ȡ����һ�����ݵģ����vPos��sizeΪ0�����õ�ǰ��m_Hands��ȡ��һ�����ݣ����vPos��size��Ϊ0��������ǰһ֡�͵�ǰ֡������ϻ�ȡvPos������//


	//��Ϊ�ǳ�ʼ֡ʱ//
	if (vPos.size() > 0) {
		float fDist = 0;
		float fMinDist = 1000;
		int iMinIndex = 0;
		cv::Point3f pKinectPos;


		//�����ù̶�����ֵ�����жϣ����������ô洢��hand���speed������ֵ���и���//
		vHandPosThreshold.resize(vCopyPreHands.size());
		for (int i = 0; i < vCopyPreHands.size(); i++) {
			vHandPosThreshold[i] = fHandPosThreshold;
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

		std::vector<cv::Point3f> vfPos;  //���ڼ�¼��ǰһ֡IDƥ���pos//
		std::vector<int> vMatchIndex;


		if (vCopyPreHands.size() < vfKinectPos.size()) {
			for (int i = 0; i < vCopyPreHands.size(); i++) {
				for (int j = 0; j < vfKinectPos.size(); j++) {

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
						pKinectPos = cv::Point3f(vfKinectPos[j].x, vfKinectPos[j].y, vfKinectPos[j].z);
						cv::Point3f pPreKinectPos = cv::Point3f(vCopyPreHands[i].worldPos.x, vCopyPreHands[i].worldPos.y, vCopyPreHands[i].worldPos.z);
						cv::Point3f distp = pKinectPos - pPreKinectPos;
						fDist = sqrt(distp.x * distp.x + distp.y * distp.y + distp.z * distp.z);
						if (fMinDist > fDist) {
							fMinDist = fDist;
							iMinIndex = j;
						}
					}

				}

				//���ڴ�����ֵ��2�����Σ�ǰһ֡Ϊ��ʧ���߳�ȥ��handPos����ǰ֡��Ӧ��handΪ������handPos,������Ҫ�Ͷ�Ӧ��Hand�ľ�����ֵ����ƥ��//
				if (fMinDist > 2 * vHandPosThreshold[i]) {
					vfPos.push_back(cv::Point3f(i, -1, -1));  //��ʾ��Ӧ��ǰһ֡Ϊ��ʧ�Ĺؼ�֡��-1��-1Ϊ��־λ//
				}

				else {
					vMatchIndex.push_back(iMinIndex);
					vfPos.push_back(cv::Point3f(vfKinectPos[iMinIndex].x, vfKinectPos[iMinIndex].y, vfKinectPos[iMinIndex].z));
				}

				fMinDist = 10000;
				iMinIndex = 0;
			}
			//����j��δ�����posֱ�Ӽӵ�vfPos�ĺ���//

			//���õ�˼��Ϊ����vMathcIndex��û�е�Index�����¶���������ƣ���Ҫ�����·����//
			for (int k = 0; k < vfKinectPos.size(); k++) {
				int l = 0;
				bool flag = true;
				while (l < vMatchIndex.size() && flag) {
					if (k == vMatchIndex[l]) {
						flag = false;
					}
					l++;
				}
				if (flag) {
					vfPos.push_back(cv::Point3f(vfKinectPos[k].x, vfKinectPos[k].y, vfKinectPos[k].z));
				}
			}
			vMatchIndex.clear();   //�κ�һ��vector����֮��һ��Ҫclear����ֹ�������ĳ������Ӱ��//
		}


		//2:ǰһ֡�����Ʊȵ�ǰ֡�����ƶ�//

		else {

			//��ʼ��vPosӦ�ú�ǰһ֡��m_Hand������ͬ��size��Ȼ�������NewHands��vfPos��������//
			vfPos.resize(vCopyPreHands.size());
			//vfPos�ĳ�ʼ������ǰһ֡�����Ƶ��ڵ�ǰ֡���ǲ����ڵ�//
			for (int i = 0; i < vfPos.size(); i++) {
				vfPos[i] = cv::Point3f(i, -1, -1);
			}


			//����1��ƥ�䣬�ҵ���Ӧ��ϵ//

			//���ڵ�ǰ֡ƥ���ϵ�����ǰһ֡�ľ��������ֵ������pos����¼���������µ�ID//
			std::vector<cv::Point3f> vCurrentNewHand;

			for (int i = 0; i < vfKinectPos.size(); i++) {
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
						pKinectPos = cv::Point3f(vfKinectPos[i].x, vfKinectPos[i].y, vfKinectPos[i].z);
						cv::Point3f pPreKinectPos = cv::Point3f(vCopyPreHands[j].worldPos.x, vCopyPreHands[j].worldPos.y, vCopyPreHands[j].worldPos.z);
						cv::Point3f distp = pKinectPos - pPreKinectPos;
						fDist = sqrt(distp.x * distp.x + distp.y * distp.y + distp.z * distp.z);
						if (fMinDist > fDist) {
							fMinDist = fDist;
							iMinIndex = j;
						}
					}
				}


				//���ڴ�����ֵ��2�����Σ�ǰһ֡Ϊ��ʧ���߳�ȥ��handPos����ǰ֡��Ӧ��handΪ������handPos,������Ҫ�Ͷ�Ӧ��Hand�ľ�����ֵ����ƥ��//
				if (fMinDist > 2 * vHandPosThreshold[iMinIndex]) {
					vCurrentNewHand.push_back(pKinectPos);                      //��¼�´�ʱ��m_Hands��pos��Ϊ�µ�����//
				}


				else {
					vfPos[iMinIndex] = cv::Point3f(vfKinectPos[i].x, vfKinectPos[i].y, vfKinectPos[i].z);
				}

				fMinDist = 10000;
				iMinIndex = 0;
			}


			//����ǰ֡��ǰһ֡ƥ�䵫�Ǵ�����ֵ��hand��pos����vfPos//
			for (int m = 0; m < vCurrentNewHand.size(); m++) {
				vfPos.push_back(vCurrentNewHand[m]);
			}
			//����ǰ֡��ǰһ֡ƥ�䵫�Ǵ�����ֵ��hand��pos����vfPos//

		}

		//���õõ���vfPos��vPos���и���//
		for (int i = 0; i < vfPos.size(); i++) {
			if (i < vPos.size()) {
				if (vPos[i].size() < 3) {
					vPos[i].push_back(vfPos[i]);
				}

				else {
					vPos[i][vPos[i].size() - 3] = vPos[i][vPos[i].size() - 2];
					vPos[i][vPos[i].size() - 2] = vPos[i][vPos[i].size() - 1];
					vPos[i][vPos[i].size() - 1] = vfPos[i];
				}
			}

			else {
				std::vector<cv::Point3f> VNewHands;
				VNewHands.push_back(vfPos[i]);
				vPos.push_back(VNewHands);
			}

		}

		//���õõ���vfPos��vPos���и���//
	}


	//  //��Ϊ��ʼ֡ʱ//                   //�Ժ�д�����ʱ�����if������Ҫ��elseһ��Ҫ��д�ϣ���ֹ�ں�������////  
	//��ʼ֡��Ӧ��vPos�㼴Ϊ��ǰ��hands��pos//
	else {
		vPos.resize(vfKinectPos.size());
		for (int i = 0; i < vPos.size(); i++) {
			vPos[i].push_back(cv::Point3f(vfKinectPos[i].x, vfKinectPos[i].y, vfKinectPos[i].z));
		}
	}

	//vPos�����ã��Ե�ǰ֡��vPos�У������Ӧ��pos��y,zΪ-1��ʾ�ڵ�ǰ֡��û�и����Ƶģ���Ҫ�Ը������Ƕ�ʧ�����뿪�����жϣ�������������һ֡�Ĵ洢�����Ƶı߽���ֵ//
	//���������ֵ�ķ�Χ֮�⣬�Ըõ�����޸����������ֵ�ķ�Χ֮�ڣ����������ж�Ϊup//

	//�ڴ˶�m_hands���е���//
	//1��������ڱ߽��ϣ������ж�Ϊup����Ϊup//
	//2��:�����in,����Ҫ����//

	float fSpeedLength = 0;
	cv::Point3f pSpeed;
	for (int i = 0; i < vPos.size(); i++) {
		if (vPos[i][vPos[i].size() - 1].y == -1 && vPos[i][vPos[i].size() - 1].z == -1) {
			bool bIsInZone = true;
			if (vPos[i].size() < 2) {

			}

			else {
				std::vector<cv::Point3f> vLastTwoPos;
				vLastTwoPos.resize(2);

				if (vPos[i].size() < 3) {
					vLastTwoPos[0] = cv::Point3f(vPos[i][0].x, vPos[i][0].y, vPos[i][0].z);
					vLastTwoPos[1] = cv::Point3f(vPos[i][0].x, vPos[i][0].y, vPos[i][0].z);
				}

				else   {
					vLastTwoPos[0] = cv::Point3f(vPos[i][0].x, vPos[i][0].y, vPos[i][0].z);
					vLastTwoPos[1] = cv::Point3f(vPos[i][1].x, vPos[i][1].y, vPos[i][1].z);
				}
				pSpeed = vLastTwoPos[1] - vLastTwoPos[0];
				fSpeedLength = sqrt(pSpeed.x * pSpeed.x + pSpeed.y * pSpeed.y + pSpeed.z * pSpeed.z);
				V3f vfPospre, vfPosAfter;
				vfPospre.x = vLastTwoPos[0].x; vfPospre.y = vLastTwoPos[0].y;  vfPospre.z = vLastTwoPos[0].z;
				vfPosAfter.x = vLastTwoPos[1].x; vfPosAfter.y = vLastTwoPos[1].y;  vfPosAfter.z = vLastTwoPos[1].z;

				V3f p = V3f(vPos[i][1].x, vPos[i][1].y, vPos[i][1].z);
				bIsInZone = IsInOrOutOfZone(p, fSpeedLength, vfPospre, vfPosAfter);
			}

			//true:��ʾ�ڱ�Ե,�ҷ����ǳ��ų�ȥ�ķ���,false��ʾ��ʶ�������м�//
			if (bIsInZone) {
				//��Ӧ�õ�Ϊup��Ҳ��vPos[i][vPos[i].size()-1]���������������־Ϊ����m_hands״̬����Ϊup//
			}
			else {

				//��һ֡������Ҳʶ�𲻵�������õ�û��ǰһ֡�����ݣ���ô�õ�͸��������ڣ�����ǰһ֡�����ݿ϶����е�//
				vPos[i][vPos[i].size() - 1] = vPos[i][vPos[i].size() - 2] + pSpeed;
			}

		}
	}

	//�Ե�ǰ��m_hand���и��� ������vPos//
	m_hands.resize(vPos.size());
	for (int i = 0; i < vPos.size(); i++) {
		m_hands[i].ID = i;
		if (vPos[i][vPos[i].size() - 1].y == -1 && vPos[i][vPos[i].size() - 1].z == -1) {
			//�����Ӧ������Ϊup,���ñ�־Ϊȷ����Ϊup��Ϊ��һ��ɾ������//
			m_hands[i].st = Up;
			m_hands[i].worldPos.x = vPos[i][vPos[i].size() - 2].x;
			m_hands[i].worldPos.y = vPos[i][vPos[i].size() - 2].y;
			m_hands[i].worldPos.z = vPos[i][vPos[i].size() - 2].z;

			//��ȡ��Ӧ����ĻͶӰ��//
			V3f p = V3f(m_hands[i].worldPos.x, m_hands[i].worldPos.y, m_hands[i].worldPos.z);
			V3f pScreenProjectp = getProjectPos(p);
			V3f vD2ScreenProjectp = convertTo(pScreenProjectp);
			m_hands[i].pos.x = vD2ScreenProjectp.x;
			m_hands[i].pos.y = vD2ScreenProjectp.y;
			m_hands[i].pos.z = vD2ScreenProjectp.z;

		}

		else  {
			V3f p = V3f(vPos[i][vPos[i].size() - 1].x, vPos[i][vPos[i].size() - 1].y, vPos[i][vPos[i].size() - 1].z);





			bool bIsInFlag = IsInZone(p);  //�����жϵ��Ƿ���ʶ�������ڣ��Ӷ��õ��õ��״̬//
			sAction act;
			act.ID = i;
			if (bIsInFlag) {
				ConvertImagePointToHand(p, act, vCopyPreHands);  //get kinect coordnitate pos//

			}

			else
			{
				act.st = Up;
				V3f pScreenProjectp = getProjectPos(p);
				V3f vD2ScreenProjectp = convertTo(pScreenProjectp);
				act.pos.x = vD2ScreenProjectp.x;
				act.pos.y = vD2ScreenProjectp.y;
				act.pos.z = vD2ScreenProjectp.z;
			}

			m_hands[i] = act;
		}
	}
	//��ȡ����ǰ֡������//

	//��һ�����ƿ��ܴ��ڵ�����ǣ���ǰ֡����Ϊup,���ǵ�ǰ֡û�����ƣ���ô���������ǻ���Ϊһ���µ����Ƴ��֣�����ڼ�������Ϊup����Ҫ��vpos������Ӧ�ĸ��£�ɾ����up��Ӧ��pos//
	std::vector<std::vector<cv::Point3f>>   vCopyvPos;
	for (int i = 0; i < m_hands.size(); i++) {
		if (m_hands[i].st != Up) {
			vCopyvPos.push_back(vPos[i]);
		}
	}
	vPos = vCopyvPos;



	//��vPos���и���//
	std::vector<std::vector<cv::Point3f>>  vCopyPos;
	for (int i = 0; i < vPos.size(); i++) {
		if (vPos[i][vPos[i].size() - 1].y != -1 || vPos[i][vPos[i].size() - 1].z != -1) {
			vCopyPos.push_back(vPos[i]);
		}
	}
	vPos = vCopyPos;

	vfKinectPos.clear();

	if (m_hands.size() == 0) {
		return (int)contours.size();
	}

	//test//
	//���ڶ������Ƶ�ȷ��//
	if (m_hands.size() >0) {
		if (vCopyPreHands.size() > 0)  {
			if (fabs(m_hands[0].worldPos.z - vCopyPreHands[0].worldPos.z) >20) {

				cv::Point3f p1 = cv::Point3f(m_hands[0].worldPos.x, m_hands[0].worldPos.y, m_hands[0].worldPos.z);
				cv::Point3f pPixelPos = cv::Point3f(getPixelPos(V3f(p1.x, p1.y, p1.z)).x, getPixelPos(V3f(p1.x, p1.y, p1.z)).y, getPixelPos(V3f(p1.x, p1.y, p1.z)).z);

				cv::Point3f p2 = cv::Point3f(vCopyPreHands[0].worldPos.x, vCopyPreHands[0].worldPos.y, vCopyPreHands[0].worldPos.z);
				cv::Point3f pPixelPos1 = cv::Point3f(getPixelPos(V3f(p2.x, p2.y, p2.z)).x, getPixelPos(V3f(p2.x, p2.y, p2.z)).y, getPixelPos(V3f(p2.x, p2.y, p2.z)).z);
			}

		}
		std::fstream out1("D:\\MoreHands.txt", std::ios::app);

		for (int i = 0; i < m_hands.size(); i++) {
			out1 << m_hands[i].ID << " " << m_hands[i].pos.x << " " << m_hands[i].pos.y << " " << m_hands[i].pos.z << " " << m_hands[i].st << " " << fSummitPoint.x << " " << fSummitPoint.y << " " << m_hands[i].worldPos.z << " " << m_hands[i].critpos.x << " " << m_hands[i].critpos.y << std::endl;
		}

		out1 << std::endl;
	}
	//���ڶ������Ƶ�ȷ��//
	//test//


	/*
	//�Ե������ƵĲ���//
	//�������Ƶ�׼ȷ��//
	else {
	std::fstream out("D:\\HandGestrue.txt", std::ios::app);
	for (int i = 0; i < m_hands.size(); i++) {
	out << m_hands[i].pos.x << " " << m_hands[i].pos.y << " " << m_hands[i].pos.z << " " << m_hands[i].st << std::endl;
	if (vPos.size() >0) {
	out << vPos[0][0].x << " " << vPos[0][0].y <<" " << vPos[0][0].z << std::endl;
	}
	}
	out << std::endl;
	}

	//�������Ƶ�׼ȷ��//
	*/
}

V3f CHandTrace::getRatio(const V3f &a, const V3f &b, const V3f & c)
{
	V3f point;

	float k11 = a.x * a.x + a.y * a.y + a.z * a.z;
	float k12 = a.x * b.x + a.y * b.y + a.z * b.z;
	float k13 = a.x * c.x + a.y * c.y + a.z *c.z;

	float k21 = a.x * b.x + a.y * b.y + a.z * b.z;
	float k22 = b.x * b.x + b.y * b.y + b.z * b.z;
	float k23 = b.x * c.x + b.y * c.y + b.z *c.z;

	point.x = (k13*k22 - k23*k12) / (k11*k22 - k21*k12);
	point.y = (k11*k23 - k21*k13) / (k11*k22 - k21*k12);
	return point;
}

V3f   CHandTrace::convertTo(const V3f& pos)
{
	V3f point1, point2, point3, a, b, c;
	point1 = vKinectPos[0];
	point2 = vKinectPos[1];
	point3 = vKinectPos[3];

	a = point2 - point1;
	b = point3 - point1;
	c = pos - point1;

	V3f K = getRatio(a, b, c);
	V3f point_1, point_2, point_3, a1, a2, a3;
	point_1 = sCreenPos[0];
	point_2 = sCreenPos[1];
	point_3 = sCreenPos[3];
	a1 = point_2 - point_1;
	a2 = point_3 - point_1;
	a3 = K.x * a1 + K.y * a2 + point_1;
	return a3;
}

V3f CHandTrace::getKinectPos(const V3f & pos)
{

	V3f p;
	p.x = (pos.x - 256) *pos.z / foc;
	p.y = (pos.y - 212)  *pos.z / foc;
	p.z = pos.z;
	return p;
}

//����Kinect�����ȡ���ص�����//
V3f CHandTrace::getPixelPos(const V3f & pos) {
	V3f p;
	p.x = pos.x * foc / pos.z + 256;
	p.y = pos.y * foc / pos.z + 212;
	return p;
}

bool CHandTrace::Precalc16_fast(cv::Mat & cv16img)
{
	for (int i = 0; i < cDepthHeight; i++) {
		for (int j = 0; j < cDepthWidth; j++) {
			V3f pImg;
			pImg.x = j;
			pImg.y = i;
			pImg.z = cv16img.ptr<USHORT>(i)[j];
			//��ʵ��ʹ���У���Kinect������Ļ�Ϸ�ʱ��ȷ������ķ�Χ����Kinect��yֵ��ȷ���ģ�������޶���Χ��ʱ�򣬲�����depth���޶�//
			//������������ֵ��ͼ��ʶ������߽�����޶�������Ļ��Kinect�Ͻ�ʱ����ʶ���y�������С��������΢����Զһ�㣬�ͻ������Ļʶ������//
			bool flag = false;
			if (pImg.z> iMinLimitz_1 && pImg.z < iMaxLimitz_1 && j > iMinLimitx_1 && j < iMaxLimitx_1) {
				V3f pImgKinectPos = getKinectPos(pImg);
				V3f projectP = getProjectPos(pImgKinectPos);
				vProToCalibPlanePos = Mult(projectP);
				if (vProToCalibPlanePos.x > m_fMinIdentX  && vProToCalibPlanePos.x < m_fMaxIdentX && vProToCalibPlanePos.y > m_fMinIdentY && vProToCalibPlanePos.y < m_fMaxIdentY)
				{
					flag = true;
				}

				if (flag) {
					float fdist = (projectP - pImgKinectPos).length();
					if (fdist < 50 || fdist > h)  {
						flag = false;
					}
				}
			}
 
			if (!flag) {
				cv16img.ptr<USHORT>(i)[j] = 0;
			}
		}
	}

	for (int i = 0; i < cDepthHeight; i++) {
		for (int j = 0; j < cDepthWidth; j++) {
			cv8Img.ptr<uchar>(i)[j] = (uchar)(cv16img.at<USHORT>(i, j) * 255.0 / iMaxLimitz);
		}
	}

	cv16Img = cv16img;  //for test//

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
Plane3f CHandTrace::GetPlane(V3f  & v1, V3f & v2, V3f & v3, V3f & v4)
{
	V3f vScreen1, vScreen2, vScreen3, vScreen4;
	vScreen1 = v1;
	vScreen2 = v2;
	vScreen3 = v3;
	vScreen4 = v4;
	Plane3f screenPlane(v1, v2, v3);

	V3f normalVector = screenPlane.normal;
	v1 = v1 - h * normalVector;
	v2 = v2 - h * normalVector;
	v3 = v3 - h * normalVector;
	v4 = v4 - h * normalVector;
	Plane3f screenPlane1(v1, v2, v3);
	return screenPlane1;
}

bool CHandTrace::IsInZone(const V3f  & pKinectPos)
{

	//��Ȼ�ǽ�������ʶ�����򣬻�����ͶӰ��ƽ���ϵ�����Ϻ�//

	V3f vfProjectScreen1, vfProjectScreen2;

	vfProjectScreen1 = getProjectPos(pKinectPos);
	vfProjectScreen2 = getProjectPos(pKinectPos, pdisScreenPlane);




	if (IsBetweenTwoPlane(vfProjectScreen1, pKinectPos, vfProjectScreen2))
	{
		if (GetPositionLocation(vKinectPos[0], vKinectPos[1], vKinectPos[2], vKinectPos[3], vfProjectScreen1))
		{
			if (GetPositionLocation(vdisMarkPoint[0], vdisMarkPoint[1], vdisMarkPoint[2], vdisMarkPoint[3], vfProjectScreen1))
				return true;
		}
	}
	return false;
}

//��ȡ��������������Ļƽ���ͶӰ�㣬��ͶӰ������ΪKinect����ϵ�µ�����//
V3f CHandTrace::getProjectPos(const V3f & sKinectPos)
{

	//test//
	if (sKinectPos.z != 0) {
		Line3f posLine;
		posLine.pos = sKinectPos;
		posLine.dir = pScreenPlane.normal;
		V3f vProjectPoint;
		pScreenPlane.intersect(posLine, vProjectPoint);
		return vProjectPoint;
	}
}

V3f CHandTrace::getProjectPos(const V3f & sKinectPos, const Plane3f & plane)
{

	//test//
	if (sKinectPos.z != 0) {
		Line3f posLine;
		posLine.pos = sKinectPos;
		posLine.dir = plane.normal;
		V3f vProjectPoint;
		plane.intersect(posLine, vProjectPoint);
		return vProjectPoint;
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

	std::vector<float>Angle;

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
		pKinectCoordinatePos = getKinectPos(pKinectCoordinatePos);
		V3f pScreenProjectPos(pKinectCoordinatePos);
		pScreenProjectPos = getProjectPos(pScreenProjectPos);
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
	//test//
	//��������//

	std::vector<int> vdelPointIndex;//��Ҫ��ɾ���ĵ�����//  


	for (int i = 0; i < contours.size(); i++)
	{


		//�ڴ˼�p�����ڱ仯��,��Ҫ�õ�����ĵ�����ֵ//
		cv::Point2f srcPoint = contours[i];

		//��ȡ��������ص�//

		cv::Point2f pCenter;   //�洢һ�����ı�־��//

		std::vector<cv::Point2f> pNeighbour;
		cv::Point2f p = contours[i];
		getValidInnerPoint(cv16Img, srcContours, p);
		//��ȡ��������ص�//

		//�ڽ�����//
		pNeighbour = getNeighbour(p);
		//�ڽ�����//

		//��ȡ�����ص����Ч���ֵ//


		/*
		//test:�۲����ֵ�Ƿ��б任//
		float preDepth = cv16Img.at<USHORT>(p.y, p.x);
		//test:�۲����ֵ�Ƿ��б任//
		*/

		bool  bIsavgDepthValid = avgDepth(p, srcContours, cv16Img);


		/*
		float afterDepth1 = cv16Img.at<USHORT>(p.y, p.x);
		*/


		if (bIsavgDepthValid) {
			cv16Img.at<USHORT>(srcPoint.y, srcPoint.x) = cv16Img.at<USHORT>(p.y, p.x);
		}

		//test:�۲����ֵ�Ƿ��б任//


		std::vector<int>vNeighbour;
		vNeighbour.resize(100);
		int iLayer = 0;

		pCenter = p;


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
						pNeighbour = getNeighbour(p);
						p = pNeighbour[vNeighbour[iLayer - 1]];

						vNeighbour[iLayer - 1] = vNeighbour[iLayer - 1] + 1;

						pNeighbour = getNeighbour(p);
						p = pNeighbour[vNeighbour[iLayer]];
						vNeighbour[iLayer] = vNeighbour[iLayer] + 1;
					}

				}
				else
				{
					vNeighbour[iLayer] = 0;
					iLayer = iLayer + 1;

					p = (pNeighbour[vNeighbour[iLayer - 1]]);
					pNeighbour = getNeighbour(p);
					vNeighbour[iLayer - 1] = vNeighbour[iLayer - 1] + 1;

				}

			}
			//����õ�����ܶ�û�кõ�//
			if (Mark == 0) {
				if (p.x > 0 && p.x < 512 && p.y > 0 && p.y < 424) {
					if (cv16Img.at<USHORT>(p.y, p.x) > 0) {
						getValidInnerPoint(cv16Img, srcContours, p);
						bIsavgDepthValid = avgDepth(p, srcContours, cv16Img);
					}
				}

				else {
					bIsavgDepthValid = false;
				}


			}
		}
		//��ȡ�����ص����Ч���ֵ//

		if (Mark == 1) {
			vdelPointIndex.push_back(i);
		}

		else {
			cv16Img.at<USHORT>(srcPoint.y, srcPoint.x) = cv16Img.at<USHORT>(p.y, p.x);
		}
	}

	std::vector<cv::Point> vCopyContours;

	if (vdelPointIndex.size() > 0) {
		for (int i = 0; i < contours.size(); i++) {
			for (int k = 0; k < vdelPointIndex.size(); k++) {
				if (i != vdelPointIndex[k]) {
					vCopyContours.push_back(contours[i]);
					break;
				}
			}
		}
	}
	else {
		vCopyContours = contours;
	}

	contours = vCopyContours;




	//test//
	//�����ֵ���в��ԣ����Ƿ��л���//

	/*
	std::fstream out20("D:\\checkDepth.txt", std::ios::app);
	for (int i = 0; i < vOridinal.size(); i++) {
	out20 << vOridinal[i] << std::endl;
	}
	out20 << std::endl;
	out20 << std::endl;
	*/


	//�����ֵ���в��ԣ����Ƿ��л���//
	//test//


	//���������//

	//��������//
	//test//
}



//ͨ�������ĵ㣬�Ըõ����Ƚ�������//

bool CHandTrace::avgDepth(const cv::Point2f & p, const std::vector<Point> & srcContours, cv::Mat & cv16Img)
{
	USHORT depth1 = 0, depth2 = 0, depth3 = 0, depth4 = 0, depth5 = 0, depth6 = 0, depth7 = 0, depth8 = 0, depth9 = 0;

	cv::Point2f  pLeft, pRight, pUp, pDown, pLeftUP, pLeftDown, pRightUp, pRightDown;


	float depth = 0;
	int Count = 1;

	std::vector<cv::Point3f> vOridinal;     //for test :�ڼ���ƽ��ֵ�Ĺ������Ƿ���ڻ���//


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

	if (pLeft.x > 0 && pLeft.x < 512 && pLeft.y >0 && pLeft.y < 424) {
		if (!isBorderOrOut(cv16Img, srcContours, pLeft)) {

			depth2 = cv16Img.at<USHORT>(pLeft.y, pLeft.x);
			depth = depth + depth2;
			Count++;
			vOridinal.push_back(Point3f(pLeft.x, pLeft.y, depth2));    //for test :�ڼ���ƽ��ֵ�Ĺ������Ƿ���ڻ���//
		}



	}


	if (pRight.x > 0 && pRight.x < 512 && pRight.y >0 && pRight.y < 424) {
		if (!isBorderOrOut(cv16Img, srcContours, pRight)) {

			depth3 = cv16Img.at<USHORT>(pRight.y, pRight.x);
			depth = depth + depth3;
			Count++;

			vOridinal.push_back(Point3f(pRight.x, pRight.y, depth3));  //for test :�ڼ���ƽ��ֵ�Ĺ������Ƿ���ڻ���//
		}


	}


	if (pUp.x > 0 && pUp.x < 512 && pUp.y >0 && pUp.y < 424) {
		if (!isBorderOrOut(cv16Img, srcContours, pUp)) {
			depth4 = cv16Img.at<USHORT>(pUp.y, pUp.x);
			depth = depth + depth4;
			Count++;

			vOridinal.push_back(Point3f(pUp.x, pUp.y, depth4));  //for test :�ڼ���ƽ��ֵ�Ĺ������Ƿ���ڻ���//
		}


	}


	if (pDown.x > 0 && pDown.x < 512 && pDown.y >0 && pDown.y < 424) {
		if (!isBorderOrOut(cv16Img, srcContours, pDown)) {
			depth5 = cv16Img.at<USHORT>(pDown.y, pDown.x);
			depth = depth + depth5;
			Count++;

			vOridinal.push_back(Point3f(pDown.x, pDown.y, depth5));  //for test :�ڼ���ƽ��ֵ�Ĺ������Ƿ���ڻ���//
		}
	}



	//��ƽ��ֵ��8���������Count����//   
	if (Count < 3) {

		if (pLeftUP.x > 0 && pLeftUP.x < 512 && pLeftUP.y >0 && pLeftUP.y < 424) {
			if (!isBorderOrOut(cv16Img, srcContours, pLeftUP)) {
				depth6 = cv16Img.at<USHORT>(pLeftUP.y, pLeftUP.x);
				depth = depth + depth6;
				Count++;
				vOridinal.push_back(Point3f(pLeftUP.x, pLeftUP.y, depth6));  //for test :�ڼ���ƽ��ֵ�Ĺ������Ƿ���ڻ���//

			}
		}

		if (pLeftDown.x > 0 && pLeftDown.x < 512 && pLeftDown.y >0 && pLeftDown.y < 424) {
			if (!isBorderOrOut(cv16Img, srcContours, pLeftDown)) {
				depth7 = cv16Img.at<USHORT>(pLeftDown.y, pLeftDown.x);
				depth = depth + depth7;
				Count++;
				vOridinal.push_back(Point3f(pLeftDown.x, pLeftDown.y, depth7)); //for test :�ڼ���ƽ��ֵ�Ĺ������Ƿ���ڻ���//

			}
		}


		if (pRightUp.x > 0 && pRightUp.x < 512 && pRightUp.y >0 && pRightUp.y < 424) {
			if (!isBorderOrOut(cv16Img, srcContours, pRightUp)) {
				depth8 = cv16Img.at<USHORT>(pRightUp.y, pRightUp.x);
				depth = depth + depth8;
				Count++;

				vOridinal.push_back(Point3f(pRightUp.x, pRightUp.y, depth8));  //for test :�ڼ���ƽ��ֵ�Ĺ������Ƿ���ڻ���//
			}
		}


		if (pRightDown.x > 0 && pRightDown.x < 512 && pRightDown.y >0 && pRightDown.y < 424) {
			if (!isBorderOrOut(cv16Img, srcContours, pRightDown)) {
				depth9 = cv16Img.at<USHORT>(pRightDown.y, pRightDown.x);
				depth = depth + depth9;
				Count++;
				vOridinal.push_back(Point3f(pRightDown.x, pRightDown.y, depth9));  //for test :�ڼ���ƽ��ֵ�Ĺ������Ƿ���ڻ���//
			}
		}

	}
	//��ƽ��ֵ��8���������Count����//


	if (Count > 2)
	{
		std::vector<float> vOridinalDepth;
		for (int i = 0; i < vOridinal.size(); i++) {
			vOridinalDepth.push_back(vOridinal[i].z);
		}

		cv::Point2f pMaxDepth = getMaxDepth(vOridinalDepth);
		float fmaxdepth = pMaxDepth.y;
		int iMaxIndex = (int)(pMaxDepth.x);

		cv::Point2f pMinDepth = getMinDepth(vOridinalDepth);
		float fmindepth = pMinDepth.y;
		int iMinIndex = (int)(pMinDepth.x);




		//����ͨ��bool���ж��Ƿ���Ҫ������vOridinalDepth���е����� ��bool Ϊfalseʱ,�������Сֵ�Ĳ�����ֵ������Χ�ڣ�������Ϊ�����������Ǻ����ݣ�����Ҫ�����������ô����ƽ��ֵ��������������ݽ�������//
		float avgDepth = depth / Count;
		while (elimateBadDepth(pMaxDepth, pMinDepth, avgDepth, vOridinalDepth) && (vOridinalDepth.size() > 2)) {
			pMaxDepth = getMaxDepth(vOridinalDepth);
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
				if (fabs(vOridinal[0].z - vOridinal[0].z) > 70) {
					return false;
				}

				else {
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
bool CHandTrace::isBorderOrOut(const cv::Mat & cv16Img, const std::vector<Point> & contours, const cv::Point & p) {
	if (cv16Img.at<USHORT>(p.y, p.x) < 10)
		return true;
	for (int i = 0; i < contours.size(); i++) {
		if (p == contours[i])
			return true;
	}
	return false;
}


cv::Point2f CHandTrace::getMaxDepth(const std::vector<float>  & vOridinalDepth) {
	int k = 0;
	float fMaxDepth = 0;
	for (int i = 0; i < vOridinalDepth.size(); i++) {
		if (vOridinalDepth[i]>fMaxDepth) {
			fMaxDepth = vOridinalDepth[i];
			k = i;
		}
	}

	return cv::Point2f(k, fMaxDepth);
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
void CHandTrace::getValidInnerPoint(const cv::Mat & cv16Img, const std::vector<Point> & srcContours, cv::Point2f & p)
{
	cv::Point2f  pLeft, pRight, pUp, pDown, pLeftUP, pLeftDown, pRightUp, pRightDown;

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


	std::vector<Point2f> pneighbour;
	pneighbour.push_back(pLeft);
	pneighbour.push_back(pRight);
	pneighbour.push_back(pUp);
	pneighbour.push_back(pDown);


	//�����ĸ������еĵ㣬Ϊ��ȡ��Ч���ڲ���//
	pneighbour.push_back(pLeftUP);
	pneighbour.push_back(pLeftDown);
	pneighbour.push_back(pRightUp);
	pneighbour.push_back(pRightDown);
	//�����ĸ������еĵ㣬Ϊ��ȡ��Ч���ڲ���//


	int m = 0;    //pneighbour���е�����//

	//�����ڲ����滻���߽��//
	for (m = 0; m < 4; m++) {
		cv::Point pTest = pneighbour[m];

		if (!isBorderOrOut(cv16Img, srcContours, pTest)) {
			p = pneighbour[m];
			break;
		}
	}

	/*
	//test//
	//�Ƿ����ĸ���Ҫô�Ǳ߽�㣬Ҫô���ⲿ��//
	std::fstream out17("D:\\fourneighbour.txt",std::ios::app);
	for (int i = 0; i < pneighbour.size(); i++) {
	out17 << pneighbour[i].x << " " << pneighbour[i].y <<" " <<cv16Img.at<USHORT>(pneighbour[i].y, pneighbour[i].x) << std::endl;
	}

	out17 << std::endl;
	out17 << std::endl;


	std::fstream out18("D:\\auxiliaryfourneighbour.txt", std::ios::app);
	for (int i = 0; i < srcContours.size(); i++) {
	out18 << srcContours[i].x << " " << srcContours[i].y << " " << cv16Img.at<USHORT>(srcContours[i].y, srcContours[i].x) << std::endl;
	}

	out18 << std::endl;
	out18 << std::endl;


	//����M����ֵ//
	std::fstream out19("D:\\auxiliaryfourneighbourGetM.txt", std::ios::app);
	out19 << m << std::endl;


	//�Ƿ����ĸ���Ҫô�Ǳ߽�㣬Ҫô���ⲿ��//

	//test result:�����ĸ��ڽ���Ҫô��ʶ�������⣬Ҫôͬ��Ϊ�߽�㣬��˿��Կ��ǽ����������8�����������������߽��Ĵ���//

	//test//
	*/

	assert(m < 8);  //�����ʱ�򲻻����ʾerror���������ʱ�����ʾerror//

}

std::vector<cv::Point2f> CHandTrace::getNeighbour(const cv::Point2f & p) {
	cv::Point2f pLeft, pRight, pUp, pDown, pLeftUP, pLeftDown, pRightUp, pRightDown;
	std::vector<cv::Point2f> pNeighbour;
	//�ڽ�����//
	pLeft = cv::Point2f(p.x - 1, p.y);
	pRight = cv::Point2f(p.x + 1, p.y);
	pUp = cv::Point2f(p.x, p.y - 1);
	pDown = cv::Point2f(p.x, p.y + 1);

	pLeftUP = cv::Point2f(p.x - 1, p.y - 1);
	pLeftDown = cv::Point2f(p.x - 1, p.y + 1);
	pRightUp = cv::Point2f(p.x + 1, p.y - 1);
	pRightDown = cv::Point2f(p.x + 1, p.y + 1);

	pNeighbour.push_back(pLeft);
	pNeighbour.push_back(pRight);
	pNeighbour.push_back(pUp);
	pNeighbour.push_back(pDown);
	pNeighbour.push_back(pLeftUP);
	pNeighbour.push_back(pLeftDown);
	pNeighbour.push_back(pRightUp);
	pNeighbour.push_back(pRightDown);
	//�ڽ�����//
	return pNeighbour;
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

	char* pstrWindowsBinaryTitle = "My binary picture";

	cvShowImage(pstrWindowsBinaryTitle, g_pBinaryImage);

	//for test//




	cv::Mat MbinaryMat(g_pBinaryImage, 0);

	findContours(MbinaryMat, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);


	cv::Mat MyMat = cv::Mat::zeros(424, 512, CV_8UC1);

	for (int i = 0; i < contours.size(); i++) {
		cv::Mat contour_mat = cv::Mat(contours[i]);

		float m = contourArea(contour_mat);

		if (contourArea(contour_mat) > 100) {
			for (int k = 0; k < contours[i].size(); k++) {
				MyMat.at<uchar>(contours[i][k].y, contours[i][k].x) = 255;
			}
		}
	}

	drawContours(MyMat, contours, 0, Scalar(255), CV_FILLED);
	imshow("savedGrayMat", MyMat);

	//for test//
}

void  CHandTrace::setPixel(int markpixelx, int markpixely) {
	this->markpixelx = markpixelx;
	this->markpixely = markpixely;
}

//��Ҫ֪��ǰһ֡���������жϵ�ǰ֡������״̬,ǰ���Ǹ����Ʊ�����ʶ��������//
//����up�㶼ɾ���ˣ���Ϊ�Ѿ���ȥ��//
//�����ҵ���ǰ������ǰһ֡��ƥ�����ƣ�//
//1�����ǰһ֡��ƥ�����ƣ��Ǹ�����Ϊdown��//
//2:���ǰһ֡����Ϊdown,���þ�������жϣ�������������ֵ,���ж�ΪMove����Ϊdown;  //�þ��������Ϊ��Ļ����//  //
//3:���ǰһ֡״̬ΪMove,��ǰ֡������ǰһ֡���������ֵ������ΪMove������Ϊnone//
//4: ���ǰһ֡������Ϊnone��˵��ǰһ֡���ڵȴ�״̬�У��������Ļ�ϵ��ƶ��������Radius,����Ϊ���Move,������Ϊnone//

//5:��������Move��ʱ��������ȴ������Լ�������������Ҳ����������趨���˶�����ֵΪ25mm������Ŀ��Ը��ݾ����������иı䣬��ǰһ֡ΪNone��ʱ���жϵľ���ı�׼Ϊ�����һ��Move�����ݣ�Ϊdownʱ���ж�Move������Ҳ�����һ֡Ϊdown������//
void CHandTrace::ConvertImagePointToHand(const V3f& p, sAction& act, const std::vector<sAction> preHands)
{
	int index = 0;
	V3f pScreenProjectp = getProjectPos(p);
	V3f pMid = convertTo(pScreenProjectp);

	cv::Point3f pKinectPos = cv::Point3f(p.x, p.y, p.z);
	bool  bHasMatchPoint = getMarchLastHandPos(preHands, pKinectPos, fHandPosThreshold, index);
	if (!bHasMatchPoint) {
		act.st = Down;
		act.critpos.x = pMid.x;
		act.critpos.y = pMid.y;
	}

	else {

		if (preHands[index].st == Move)
		{
			act.critpos = preHands[index].pos;
		}
		else {
			act.critpos = preHands[index].critpos;
		}


		float distance = sqrt((pMid.x - act.critpos.x) * (pMid.x - act.critpos.x) + (pMid.y - act.critpos.y) * (pMid.y - act.critpos.y));
		if (preHands[index].st == Down) {
			if (distance > Radius) {
				act.st = Move;
			}
			else {
				act.st = Down;
			}

		}

		else if (preHands[index].st == Move) {
			if (distance > Radius) {
				act.st = Move;
			}
			else {
				act.st = None;
			}
		}

		else if (preHands[index].st == None){
			if (distance > Radius) {
				act.st = Move;
			}
			else {
				act.st = None;
			}
		}

		if (preHands.size() == 0) {
			if (act.st == Down && preHands[0].st == None) {
			}

		}

	}



	//��ȡ��ǰ���Ƶ���Ļ����//
	//�����ǰ֡��Ӧ������ΪDown��ǰһ֡��Ӧ������ҲΪDown���ǵ�ǰ֡��ScreenPos��ǰһ֡�����worldPosҲ��ǰһ֡���//
	/*
	if (bHasMatchPoint) {
	if ((act.st == Down && preHands[index].st == Down) || (act.st == None  && preHands[index].st == Move) || (act.st == None  && preHands[index].st == None))
	{
	act.pos.x = preHands[index].pos.x;
	act.pos.y = preHands[index].pos.y;
	act.pos.z = preHands[index].pos.z;

	//��ȡ��ǰ���Ƶ�ȫ������//
	act.worldPos.x = preHands[index].worldPos.x;
	act.worldPos.y = preHands[index].worldPos.y;
	act.worldPos.z = preHands[index].worldPos.z;
	}


	else {
	act.pos.x = pMid.x;
	act.pos.y = pMid.y;
	act.pos.z = pMid.z;

	//��ȡ��ǰ���Ƶ�ȫ������//
	act.worldPos.x = p.x;
	act.worldPos.y = p.y;
	act.worldPos.z = p.z;
	}
	}

	*/

	//	else {
	act.pos.x = pMid.x;
	act.pos.y = pMid.y;
	act.pos.z = pMid.z;

	//��ȡ��ǰ���Ƶ�ȫ������//
	act.worldPos.x = p.x;
	act.worldPos.y = p.y;
	act.worldPos.z = p.z;
	//	}


	//�������ֵ�ı仯����Ƿ��ȶ�//
	// 		std::fstream out2("D:\\DepthChangeTest.txt", std::ios::app);
	// 		out2 << p.x << " " << p.y <<" "<< p.z << std::endl;

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

	V3f ImgKinectPos, ImgProjectPos, ImgScreenPos ,  v3fCalibPlanePos;
	std::vector<V3f>  vCalibPlaneCoordinatePos;
	for (int i = 0; i < srcContours.size(); i++) {
		ImgKinectPos = getKinectPos(V3f(srcContours[i].x, srcContours[i].y, cv16img.at<USHORT>(srcContours[i].y, srcContours[i].x)));
		ImgProjectPos = getProjectPos(ImgKinectPos);

		//��ͼ��λ��ת�����궨������ϵ��//
		v3fCalibPlanePos = Mult(ImgProjectPos);
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

	std::vector<float> vMinDist;
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
	esummitdir = (eDir)(iGlobalMinIndex);
	//����ֵ�����жϣ�ȷ���ú����Ƿ���ã������ã��ж����Ƶı߽�����//




	switch (esummitdir) {
	case left: {
		fSummitPoint = srcContours[iMaxIndey];
		break;
	}

	case right: {
		fSummitPoint = srcContours[iMinIndey];
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

	// vKinectPos��ǵ��ĸ��㣻vdistMarkPoint:�������ĸ��� 
	V3f vMark1, vMark2, vMark3, vMark4, vDistMark1, vDistMark2, vDistMark3, vDistMark4;

	vMark1 = V3f(vKinectPos[0].x, vKinectPos[0].y, vKinectPos[0].z);
	vMark2 = V3f(vKinectPos[1].x, vKinectPos[1].y, vKinectPos[1].z);
	vMark3 = V3f(vKinectPos[2].x, vKinectPos[2].y, vKinectPos[2].z);
	vMark4 = V3f(vKinectPos[3].x, vKinectPos[3].y, vKinectPos[3].z);

	vDistMark1 = V3f(vdisMarkPoint[0].x, vdisMarkPoint[0].y, vdisMarkPoint[0].z);
	vDistMark2 = V3f(vdisMarkPoint[1].x, vdisMarkPoint[1].y, vdisMarkPoint[1].z);
	vDistMark3 = V3f(vdisMarkPoint[2].x, vdisMarkPoint[2].y, vdisMarkPoint[2].z);
	vDistMark4 = V3f(vdisMarkPoint[3].x, vdisMarkPoint[3].y, vdisMarkPoint[3].z);

	p1 = Plane3f(vDistMark1, vDistMark2, vDistMark3);
	p2 = Plane3f(vMark1, vDistMark1, vDistMark4);
	p3 = Plane3f(vMark2, vDistMark2, vMark3);
	p4 = Plane3f(vDistMark3, vDistMark3, vMark4);
	p5 = Plane3f(vMark1, vDistMark1, vMark2);


	std::vector<Plane3f> vPlane;
	vPlane.resize(5);

	for (int i = 0; i < vPlane.size(); i++) {
		CalDistFromPointToPlane(KinectPos, vPlane[i], fPosDist[i]);
	}

	if (fSpeedLength > 0.1) {
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
	}

	else {
		vDir.resize(vPlane.size());
		for (int i = 0; i < vPlane.size(); i++) {
			vDir[i] = 0;
		}
	}

	//�����һ�����޷�ʶ�𵽣��жϸõ��Ƿ�����Ϊ����ʶ�����������//


	for (int i = 0; i < vDir.size(); i++) {
		if (vDir[i] == 1) {
			if (fPosDist[i] < fOutThreshold) {
				return true;

			}

		}
	}
	return false;
	//�����һ�����޷�ʶ�𵽣��жϸõ��Ƿ�����Ϊ����ʶ�����������//

}

//���ں�ǰһ֡�����ƽ���ƥ��//
bool  CHandTrace::getMarchLastHandPos(const std::vector<sAction>  & vLastHand, const cv::Point3f  & kinectPos, const float & threshold, int  & index)
{

	float fMinDist = 10000;
	float fDist;
	int iMinIndex = 0;
	for (int i = 0; i < vLastHand.size(); i++) {
		cv::Point3f pDist = cv::Point3f(vLastHand[i].worldPos.x - kinectPos.x, vLastHand[i].worldPos.y - kinectPos.y, vLastHand[i].worldPos.z - kinectPos.z);
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
	if (KinectInstallPattern == 0)
	{
		if (a > b)
			return true;
		return false;
	}
	else if (KinectInstallPattern == 1) {
		if (a < b)
			return true;
		return false;
	}
}

//��ȡyֵ��Сʱ�Ķ�Ӧ��ָ��//
int CHandTrace::GetMinIndex(std::vector<cv::Point> &vGroups)
{
	int index = 0;
	float minPixely = 10000;
	for (int i = 0; i < vGroups.size(); i++) {
		if (minPixely < vGroups[i].y)
			index = i;
	}
	return index;
}

int CHandTrace::GetMaxIndex(std::vector<cv::Point> &vGroups)
{
	int index = 0;
	float maxPixely = 0;
	for (int i = 0; i < vGroups.size(); i++) {
		if (minPixely > vGroups[i].y)
			index = i;
	}
	return index;
}

//1������yֵ���ж����Ƽ�˵�//
cv::Point2f CHandTrace::getSummitPoint(std::vector<cv::Point> & contours) {

	if (contours.size() == 1) {
		return contours[0];
	}

	else if (contours.size() == 2) {
		float pixely1 = contours[0].y;
		float pixely2 = contours[1].y;
		if (GetSummitPoint(pixely1, pixely2))  
			return contours[0];
		return contours[1];

	}

	else  {
		int indexMin = GetMinIndex(contours);
		int indexMax = GetMaxIndex(contours);
		if (GetSummitPoint(contours[indexMin].y, contours[indexMax].y))
			return contours[indexMin].y;
		return contours[indexMax].y;
	}
}



std::vector<std::vector<cv::Point>>  CHandTrace::getGroup(const int & k, const std::vector<cv::Point>  & srcContours, const float  & threshold) {
	std::vector<std::vector<cv::Point>> vGroups;
	vGroups.resize(k);
	std::vector<cv::Point> vMarkPoint;
	vMarkPoint.resize(k);
	//���ó�ʼ��center��//
	for (int i = 0; i < k; i++) {
		vMarkPoint[i] = srcContours[i];
	}
	//���ó�ʼ��center��//

	std::vector<cv::Point> vLastPoint;
	vLastPoint = vMarkPoint;
	std::vector<float> vCenterChangeDist;
	vCenterChangeDist.resize(k);
	for (int i = 0; i < k; i++) {
		vCenterChangeDist[i] = 10000;
	}

	float fMinCenterChangeDist = getMin(vCenterChangeDist);

	while (fMinCenterChangeDist > threshold) {
		UpdateGroup(vMarkPoint, srcContours, vGroups);
		for (int i = 0; i < k; i++) {
			vMarkPoint[i] = getOptimalPoint(vGroups[i]);
			cv::Point2f pDist(vMarkPoint[i] - vLastPoint[i]);
			vCenterChangeDist[i] = sqrt(pDist.x * pDist.x + pDist.y * pDist.y);
			vLastPoint[i] = vMarkPoint[i];
		}

		fMinCenterChangeDist = getMin(vCenterChangeDist);
	}
	return vGroups;

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

//belong to K-Means Algorithm//
//��ȡ�����е�ľ������С�ĵ�//
cv::Point CHandTrace::getOptimalPoint(const std::vector<cv::Point> & srcContours) {
	float favgx = 0, favgy = 0;
	cv::Point2f optimalPoint;
	for (int i = 0; i < srcContours.size(); i++) {
		favgx = favgx + srcContours[i].x;
		favgy = favgy + srcContours[i].y;
	}

	optimalPoint.x = (favgx / (srcContours.size()));
	optimalPoint.y = (favgy / (srcContours.size()));
	return optimalPoint;
}
//��ȡ�����е�ľ������С�ĵ�//
//belong to K-Means Algorithm//

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


std::vector<int>  CHandTrace::TestGetUseTime()
{
	std::vector<int> vUseTime;
	vUseTime.push_back(useTime1);
	vUseTime.push_back(useTime2);
	return vUseTime;
}


float CHandTrace::TestGetSummity()
{
	return m_fSummity;
}


void CHandTrace::TestSetSummity(float fSummity)
{
	m_fSummity = fSummity;
}


bool CHandTrace::SetKinectCoordinateSystem()
{
	V3f v1 = vKinectPos[0];
	V3f v2 = vKinectPos[1];
	V3f v3 = vKinectPos[3];
	V3f vRad1 = (v2 - v1).normalize();
	V3f vRad2 = (v3 - v1).normalize();
	V3f vNormVector = (vRad1.cross(vRad2)).normalize();
	Quat<float> q;

	
	q.setAxisAngle(vNormVector, 1.576786327);
	V3f vAxisY = vRad1 * q;
	V3f vAxisX = vRad1;
	V3f vAxisZ = vNormVector;
	V3f vTransVec = v1;

	Matrix44<float> _invTransMat(1, 0, 0, -vTransVec.x,0,1,0,-vTransVec.y,0,0,1,-vTransVec.z,0,0,0,1);
	Matrix44<float> _invRotMat(vAxisX.x, vAxisX.y, vAxisX.z, 0, vAxisY.x, vAxisY.y, vAxisY.z, 0, vAxisZ.x, vAxisZ.y, vAxisZ.z, 0, 0, 0, 0, 1);

	invTransFromMat = _invRotMat * _invTransMat;
	return true;
}


V3f CHandTrace::Mult(V3f vec)
{
	V4f v1(vec.x,vec.y,vec.z, 1);
	V3f v;
	v.x = invTransFromMat[0][0] * v1.x + invTransFromMat[0][1] * v1.y + invTransFromMat[0][2] * v1.z + invTransFromMat[0][3] * v1.w;
	v.y = invTransFromMat[1][0] * v1.x + invTransFromMat[1][1] * v1.y + invTransFromMat[1][2] * v1.z + invTransFromMat[1][3] * v1.w;
	v.z = invTransFromMat[2][0] * v1.x + invTransFromMat[2][1] * v1.y + invTransFromMat[2][2] * v1.z + invTransFromMat[2][3] * v1.w;
	return v;
}

void CHandTrace::GetInentZone()
{
	V3f v1 = Mult(vKinectPos[0]);
	V3f v2 = Mult(vKinectPos[1]);
	V3f v3 = Mult(vKinectPos[2]);
	V3f v4 = Mult(vKinectPos[3]);
	m_fMaxIdentX = max(max(v1.x,v2.x), max(v3.x,v4.x));
	m_fMinIdentX = min(min(v1.x, v2.x), min(v3.x, v4.x));
	m_fMaxIdentY = max(max(v1.y, v2.y), max(v3.y, v4.y));
	m_fMinIdentY = min(min(v1.y, v2.y), min(v3.y, v4.y));
}


void CHandTrace::GetReviseHandPos()
{
	if (vfKinectPos.size() == 0)
		return;
	//��С��Ϊͼ��ʶ������������������е��������ƾ���Ͻ��޳��������������  begin://
	for (int i = 0; i < (vfKinectPos.size() - 1); i++) {
		for (int j = i + 1; j < vfKinectPos.size(); j++) {
			V2f vdistVec = V2f((vfKinectPos[j] - vfKinectPos[i]).x, (vfKinectPos[j] - vfKinectPos[i]).y);
			float fdist = vdistVec.length();
			if (fdist < m_fHandThreshold) {
				if (j != vfKinectPos.size() - 1) {
					for (int k = j; k < vfKinectPos.size() - 1; k++){
						vfKinectPos[k] = vfKinectPos[k + 1];
					}
				}
				else {
					vfKinectPos.pop_back();
				}
			}

		}
	}

	//:end//
}

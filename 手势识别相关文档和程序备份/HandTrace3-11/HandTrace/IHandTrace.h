#include "iostream"
#include "vector"
#include "IDLPGesture.h"
#include <fstream>
#include <iostream>
#include <vector>
#include "opencv/cv.h"
#include "opencv2/highgui/highgui.hpp"
#include "opencv/highgui.h"
#include "opencv/cv.h"

#include "OpenExr/include/ImathLine.h"
#include "OpenExr/include/ImathLineAlgo.h"
#include "OpenExr/include/ImathPlane.h"
#include "OpenExr/include/ImathQuat.h"
using namespace IMATH_NAMESPACE;
using namespace std;
using namespace cv;



namespace HandTrace
{
	__interface IHandTrace {
		void InitData(const std::vector<V3f> & vmarkscreenpos, const std::vector<V3f> & vmarkKinectPos, int KinectInstallPattern,float h, float h1, float Radius);
		std::vector<sAction> GetHandGesture(cv::Mat &cv16Img);

		//test//
		std::vector<cv::Point2f> GetCounterSummitPoint();
		void SetVidient();
		std::vector<int> TestGetUseTime();
		float TestGetSummity();
		std::vector<cv::Point3f> GetCounterSummitPoint3();
		int GetHandNum();
	};

	typedef std::shared_ptr<IHandTrace> PIHandTrace;
	PIHandTrace createPIHandTrace();
}
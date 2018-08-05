// HandTrace.h : HandTrace DLL 的主头文件
//

#pragma once
#include "opencv\cv.h"
using namespace cv;


#ifdef _DEBUG
#pragma comment(lib,"opencv_calib3d248d.lib")
#pragma comment(lib,"opencv_core248d.lib")
#pragma comment(lib,"opencv_highgui248d.lib")
#pragma comment(lib,"opencv_video248d.lib")
#pragma comment(lib,"opencv_ml248d.lib")
#pragma comment(lib,"opencv_legacy248d.lib")
#pragma comment(lib,"opencv_imgproc248d.lib")
#pragma comment(lib,"opencv_flann248d.lib")
#pragma comment(lib,"opencv_features2d248d.lib")
#pragma comment(lib,"zlibd.lib")
#pragma comment(lib,"libjasperd.lib")
#pragma comment(lib,"libjpegd.lib")
#pragma comment(lib,"libpngd.lib")
#pragma comment(lib,"libtiffd.lib")

#else 

#pragma comment(lib,"opencv_calib3d248.lib")
#pragma comment(lib,"opencv_core248.lib")
#pragma comment(lib,"opencv_highgui248.lib")
#pragma comment(lib,"opencv_video248.lib")
#pragma comment(lib,"opencv_ml248.lib")
#pragma comment(lib,"opencv_legacy248.lib")
#pragma comment(lib,"opencv_imgproc248.lib")
#pragma comment(lib,"opencv_flann248.lib")
#pragma comment(lib,"opencv_features2d248.lib")
#pragma comment(lib,"zlib.lib")
#pragma comment(lib,"libjasper.lib")
#pragma comment(lib,"libjpeg.lib")
#pragma comment(lib,"libpng.lib")
#pragma comment(lib,"libtiff.lib")


#endif

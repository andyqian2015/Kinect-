#pragma  once
#ifndef CMATHBASE_H
#define CMATHBASE_H


#include "dlpgesture/IDLPGesture.h"
#include "OpenExr/include/ImathVec.h"
#include "OpenExr/include/ImathPlane.h"
using namespace IMATH_NAMESPACE;


#define foc 370




class CMathBase : public IMathBase
{
public:
	void  getKinectPos(const V3f & pos, V3f & Point, int DepthWidth, int DepthHeight);
	void  getPixelPos(const V3f & pos, V3f &pixelPoint, int DepthWidth, int DepthHeight);
	void  getProjectPos(const Plane3f &ScreenPlane, const V3f & sKinectPos, V3f & vProjectPoint);
	

};




#endif
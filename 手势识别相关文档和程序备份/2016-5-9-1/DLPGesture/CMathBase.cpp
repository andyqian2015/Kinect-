
#include "stdafx.h"
#include "CMathBase.h"

PIMathBase GestureEng::createPIMathBase()
{
	PIMathBase p(new CMathBase);
	return p;
}

void  CMathBase::getKinectPos(const V3f & pos, V3f & Point,int DepthWidth,int DepthHeight)
{
	Point.x = (pos[0] - DepthWidth / 2) *pos[2] / foc;
	Point.y = (pos[1] - DepthHeight / 2)  *pos[2] / foc;
	Point.z = pos[2];
}


void CMathBase::getPixelPos(const V3f & pos, V3f &pixelPoint, int DepthWidth, int DepthHeight)
{
	pixelPoint.x = pos.x * foc / pos.z + DepthWidth / 2;
	pixelPoint.y = pos.y * foc / pos.z + DepthHeight / 2;
	pixelPoint.z = pos.z;
}

void CMathBase::getProjectPos(const Plane3f &ScreenPlane, const V3f & sKinectPos, V3f & vProjectPoint)
{
	if (sKinectPos.z != 0) {
		Line3f posLine;
		posLine.pos = sKinectPos;
		posLine.dir = ScreenPlane.normal;
		ScreenPlane.intersect(posLine, vProjectPoint);
	}
}


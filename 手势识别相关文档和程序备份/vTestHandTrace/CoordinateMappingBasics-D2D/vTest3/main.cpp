#include "iostream"
#include "vector"
#include "atlbase.h"
#include <memory>
#include "Windows.h"
#include "ICalib.h"
#include "OpenExr/include/ImathVec.h"
using namespace IMATH_NAMESPACE;
void __stdcall Test(std::vector<sAction> &HandGesture)
{
	ATLTRACE("%d\n", HandGesture.size());
	for (int i = 0; i < HandGesture.size(); i++){
		ATLTRACE("%d\n", HandGesture[i].ID);
	}
}


int APIENTRY wWinMain(
	_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR lpCmdLine,
	_In_ int nShowCmd
	)
{
	//获取标定参数//

	V3f vscreenpos1(0, 0, 0);
	V3f vscreenpos2(0, 650, 0);
	V3f vscreenpos3(920, 650, 0);
	V3f vscreenpos4(920, 0, 0);
	std::vector<V3f> vMarkScreenPos;
	vMarkScreenPos.push_back(vscreenpos1);
	vMarkScreenPos.push_back(vscreenpos2);
	vMarkScreenPos.push_back(vscreenpos3);
	vMarkScreenPos.push_back(vscreenpos4);

	float h = 500, h1 = 100, Radius = 25;
	int KinectInstallPattern = 0;

	std::shared_ptr<ICalibPro::ICalib> PTestCalib = ICalibPro::CreatePICalib();
 	PTestCalib->Run(hInstance, nShowCmd);
 	PTestCalib->getMarkKinectPos();
	std::vector<V3f> vMarkKinectPos;
	vMarkKinectPos = PTestCalib->getMarkKinectPos();

	std::shared_ptr<IGetHandGesturePro::IGetHandGesture> PTest = IGetHandGesturePro::createPIGetHandGesture(vMarkScreenPos, vMarkKinectPos, KinectInstallPattern, h, h1, Radius);
 	PTest->Registry((IGetHandGesturePro::CallBack)Test);
 	PTest->Run(hInstance, nShowCmd);
	return 0;
}
#include "CGetHands.h"


namespace NAMESPACE_HANDGESTURE {
	PIGetHands createPIGetHands(HINSTANCE hInstance, int nCmdShow) {
		PIGetHands p(new CGetHands(hInstance, nCmdShow));
		return p;
	}
}


CGetHands::CGetHands(HINSTANCE hInstance, int nCmdShow)
{

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

	m_PICalib = ICalibPro::CreatePICalib();
	m_PICalib->Run(hInstance, nCmdShow);
	std::vector<V3f> vMarkKinectPos;
	vMarkKinectPos = m_PICalib->getMarkKinectPos();
	m_PIGetHandGesture = IGetHandGesturePro::createPIGetHandGesture(vMarkScreenPos, vMarkKinectPos, KinectInstallPattern, h, h1, Radius);
}

void CGetHands::fGetHands(HINSTANCE hInstance, int nCmdShow)
{
	m_PICalib->Run(hInstance, nCmdShow);
	m_PICalib->getMarkKinectPos();
	m_CallBack = std::bind(&CGetHands::Test, this, std::placeholders::_1);
	m_PIGetHandGesture->Registry(m_CallBack);
 	m_PIGetHandGesture->Run(hInstance, nCmdShow);
}


void _stdcall CGetHands::Test(std::vector<sAction> HandsGesture)
{
	std::fstream out("D:\\Test.txt", std::ios::app);
	for (int i = 0; i < HandsGesture.size(); i++) {
		out << HandsGesture[i].ID << " " << HandsGesture[i].pos.x << " " << HandsGesture[i].pos.y << " " << HandsGesture[i].pos.z << " " << HandsGesture[i].st << " " << " " << HandsGesture[i].worldPos.z << " " << HandsGesture[i].critpos.x << " " << HandsGesture[i].critpos.y << std::endl;
	}
}

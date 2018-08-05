#include "IHandGesture.h"
#include "fstream"
#include "ITestPreInterface.h"
#include "atlbase.h"
#include "ICalib.h"

class CGetHands : public NAMESPACE_HANDGESTURE::IGetHands
{
public :
	CGetHands(HINSTANCE hInstance, int nCmdShow);
	void fGetHands(HINSTANCE hInstance, int nCmdShow);
	void _stdcall Test(std::vector<sAction> HandsGesture);

private:
	std::shared_ptr<IGetHandGesturePro::IGetHandGesture> m_PIGetHandGesture;
	IGetHandGesturePro::CallBack m_CallBack;
	std::shared_ptr<IGetHandGesturePro::CallBack> m_PICallBack;

	ICalibPro::PICalib  m_PICalib;//标定的接口//
};
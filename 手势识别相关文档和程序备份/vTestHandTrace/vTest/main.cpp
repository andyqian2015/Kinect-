#include "iostream"
#include "ICalib.h" 
void __stdcall Test(std::vector<sAction> handsgesture);

int main()
{
	std::shared_ptr<IGetHandGesturePro::IGetHandGesture> PTest = IGetHandGesturePro::createPIGetHandGesture();
	std::vector<sAction> handsgesture;
	PTest->Registry((IGetHandGesturePro::CallBack)Test(handsgesture));
	PTest->Run();
}
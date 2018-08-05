 #include "IHandGesture.h"
 #include <memory>
#include "iostream"


#include "Windows.h"
#include  "atlbase.h"
int APIENTRY wWinMain(
	_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR lpCmdLine,
	_In_ int nShowCmd
	)
{
	std::shared_ptr<NAMESPACE_HANDGESTURE::IGetHands> PIGetHands = NAMESPACE_HANDGESTURE::createPIGetHands(hInstance, nShowCmd);
 	PIGetHands->fGetHands(hInstance, nShowCmd);
	return 0;
}
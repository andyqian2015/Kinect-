#include "ITestPreInterface.h"

class CTestPreInterface :public NAMESPACE_TESTPRE::ITestPreInterface {
public :
	int Run(HINSTANCE hInstance, int nCmdShow);
	void Registry(NAMESPACE_TESTPRE::CallBack callback);

private:
	NAMESPACE_TESTPRE::CallBack  m_fun;
};
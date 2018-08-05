#include "CTestPreInterface.h"

namespace NAMESPACE_TESTPRE
{
	PITestPreInterface createPITestPreInterface()
	{
		PITestPreInterface p(new CTestPreInterface);
		return p;
	}
}

int CTestPreInterface::Run(HINSTANCE hInstance, int nCmdShow)
{
	std::vector<int> vec(3);
	for (int i = 0; i < vec.size(); i++) {
		vec[i] = i;
	}
	m_fun(vec);
	return 0;
}
void CTestPreInterface::Registry(NAMESPACE_TESTPRE::CallBack callback)
{
	m_fun = callback;
}
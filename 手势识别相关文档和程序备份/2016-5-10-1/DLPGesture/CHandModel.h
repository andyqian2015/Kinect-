//单手父类 包含基础手势
#ifndef CHANDMODEL_H
#define CHANDMODEL_H

class CHandModel
{
public:
	virtual void Click(){};
	virtual void MoveTo(){};
	virtual void LongPush(){};
};
#endif
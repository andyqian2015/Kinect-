#ifndef QSETPARAMWND_H
#define QSETPARAMWND_H

#include <QtWidgets/QDialog>
#include "ui_QSetParamwnd.h"
#include "GestureParam.h"

class QSetParamWnd : public QDialog
{
	Q_OBJECT

public:
	QSetParamWnd(GestureEng::PIGestureParam gesturepointer,QWidget *parent = 0);
	~QSetParamWnd();


private:
	Ui::QSetParamWndClass ui;

	GestureEng::PIGestureParam pparam;


private slots:
void setbasic();
void setclick();
void setdclick();
void setflip();
void setslideback();
void setzar();
void saveall();
void reset();

void clickdisable(bool);
void dclickdisable(bool);
void flipupdisable(bool);
void flipdowndisable(bool);
void flipleftdisable(bool);
void fliprightdisable(bool);
void lpushdisable(bool);
void switchhoridisable(bool);
void switchvertdisable(bool);
void zoomdisable(bool);
void rotatedisable(bool);
void dragdisable(bool);
void movetodisable(bool);
};

#endif // QSETPARAMWND_H

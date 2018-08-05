#include "stdafx.h"
#include "QSetParamwnd.h"

QSetParamWnd::QSetParamWnd(GestureEng::PIGestureParam gesturepointer,QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);
	if (!gesturepointer)
	{
		//弹出错误信息
		//安全退出	
		exit(1);
	}
	pparam=gesturepointer;
	pparam->GetParamValue();

	ui.b_radiusspin->setValue(pparam->PassBasicParam()->radius_move);
	ui.b_timespin->setValue(pparam->PassBasicParam()->time_max);
	ui.l_timespin->setValue(pparam->PassBasicParam()->time_longpush);
	ui.c_spin->setValue(pparam->PassClickParam()->radius_click);
	ui.d_timespin->setValue(pparam->PassDClickParam()->time_dclick);
	ui.d_scopespin->setValue(pparam->PassDClickParam()->radius_dclick);
	ui.f_speedspin->setValue(pparam->PassFlipParam()->speed_flip);
	ui.f_distspin->setValue(pparam->PassFlipParam()->distance_flip);
	ui.s_distspin->setValue(pparam->PassBackParam()->distance_slide);
	ui.s_bacdspin->setValue(pparam->PassBackParam()->distance_back);
	ui.s_percentspin->setValue((int)(pparam->PassBackParam()->proportion_back * 100));
	ui.z_rotatespin->setValue((int)(pparam->PassZARParam()->angle_rotate * 100));
	ui.z_zoomspin->setValue((int)(pparam->PassZARParam()->proportion_zoom * 100));

	ui.moveto->setChecked(pparam->GetChecked(GMoveTo));
	ui.click_2->setChecked(pparam->GetChecked(GClick));
	ui.lpush->setChecked(pparam->GetChecked(GLongPush));
	ui.dclick_2->setChecked(pparam->GetChecked(GDoubleClick));
	ui.flipright->setChecked(pparam->GetChecked(GFlipRight));
	ui.flipleft->setChecked(pparam->GetChecked(GFlipLeft));
	ui.switchhori->setChecked(pparam->GetChecked(GSwithHori));
	ui.flipup->setChecked(pparam->GetChecked(GFlipUp));
	ui.flipdown->setChecked(pparam->GetChecked(GFlipDown));
	ui.switchvert->setChecked(pparam->GetChecked(GSwithVert));
	ui.drag->setChecked(pparam->GetChecked(GDrag));
	ui.zoom->setChecked(pparam->GetChecked(GZoom));
	ui.rotate->setChecked(pparam->GetChecked(GRotate));


	QObject::connect(ui.okButton, SIGNAL(clicked()), this, SLOT(saveall()));
	QObject::connect(ui.applyButton, SIGNAL(clicked()), this, SLOT(saveall()));
	QObject::connect(ui.resetButton, SIGNAL(clicked()), this, SLOT(reset()));

	QObject::connect(ui.click_2, SIGNAL(clicked(bool)), this, SLOT(clickdisable(bool)));
	QObject::connect(ui.dclick_2, SIGNAL(clicked(bool)), this, SLOT(dclickdisable(bool)));
	QObject::connect(ui.moveto, SIGNAL(clicked(bool)), this, SLOT(movetodisable(bool)));
	QObject::connect(ui.lpush, SIGNAL(clicked(bool)), this, SLOT(lpushdisable(bool)));
	QObject::connect(ui.flipdown, SIGNAL(clicked(bool)), this, SLOT(flipdowndisable(bool)));
	QObject::connect(ui.flipup, SIGNAL(clicked(bool)), this, SLOT(flipupdisable(bool)));
	QObject::connect(ui.flipleft, SIGNAL(clicked(bool)), this, SLOT(flipleftdisable(bool)));
	QObject::connect(ui.flipright, SIGNAL(clicked(bool)), this, SLOT(fliprightdisable(bool)));
	QObject::connect(ui.drag, SIGNAL(clicked(bool)), this, SLOT(dragdisable(bool)));
	QObject::connect(ui.switchhori, SIGNAL(clicked(bool)), this, SLOT(switchhoridisable(bool)));
	QObject::connect(ui.switchvert, SIGNAL(clicked(bool)), this, SLOT(switchvertdisable(bool)));
	QObject::connect(ui.zoom, SIGNAL(clicked(bool)), this, SLOT(zoomdisable(bool)));
	QObject::connect(ui.rotate, SIGNAL(clicked(bool)), this, SLOT(rotatedisable(bool)));

}

QSetParamWnd::~QSetParamWnd()
{

}

void QSetParamWnd::reset()
{
	pparam->ResetParam();
	pparam->GetParamValue();

	ui.b_radiusspin->setValue(pparam->PassBasicParam()->radius_move);
	ui.b_timespin->setValue(pparam->PassBasicParam()->time_max);
	ui.l_timespin->setValue(pparam->PassBasicParam()->time_longpush);
	ui.c_spin->setValue(pparam->PassClickParam()->radius_click);
	ui.d_timespin->setValue(pparam->PassDClickParam()->time_dclick);
	ui.d_scopespin->setValue(pparam->PassDClickParam()->radius_dclick);
	ui.f_speedspin->setValue(pparam->PassFlipParam()->speed_flip);
	ui.f_distspin->setValue(pparam->PassFlipParam()->distance_flip);
	ui.s_distspin->setValue(pparam->PassBackParam()->distance_slide);
	ui.s_bacdspin->setValue(pparam->PassBackParam()->distance_back);
	ui.s_percentspin->setValue((int)(pparam->PassBackParam()->proportion_back * 100));
	ui.z_rotatespin->setValue((int)(pparam->PassZARParam()->angle_rotate * 100));
	ui.z_zoomspin->setValue((int)(pparam->PassZARParam()->proportion_zoom * 100));

	ui.moveto->setChecked(pparam->GetChecked(GMoveTo));
	ui.click_2->setChecked(pparam->GetChecked(GClick));
	ui.lpush->setChecked(pparam->GetChecked(GLongPush));
	ui.dclick_2->setChecked(pparam->GetChecked(GDoubleClick));
	ui.flipright->setChecked(pparam->GetChecked(GFlipRight));
	ui.flipleft->setChecked(pparam->GetChecked(GFlipLeft));
	ui.switchhori->setChecked(pparam->GetChecked(GSwithHori));
	ui.flipup->setChecked(pparam->GetChecked(GFlipUp));
	ui.flipdown->setChecked(pparam->GetChecked(GFlipDown));
	ui.switchvert->setChecked(pparam->GetChecked(GSwithVert));
	ui.drag->setChecked(pparam->GetChecked(GDrag));
	ui.zoom->setChecked(pparam->GetChecked(GZoom));
	ui.rotate->setChecked(pparam->GetChecked(GRotate));
}

void QSetParamWnd::setbasic()
{
	pparam->SetGestureParam(ui.b_timespin->value(), ui.b_radiusspin->value(),ui.l_timespin->value());
}

void QSetParamWnd::setclick()
{
	pparam->SetGestureParamClick(ui.c_spin->value());
}

void QSetParamWnd::setdclick()
{
	pparam->SetGestureParamDClick(ui.d_timespin->value(), ui.d_scopespin->value());
}

void QSetParamWnd::setflip()
{
	pparam->SetGestureParamFlip(ui.f_speedspin->value(), ui.f_distspin->value());
}

void QSetParamWnd::setslideback()
{
	double temp = ui.s_percentspin->value();
	double prop = temp/100;
	pparam->SetGestureParamSlideBack(prop, ui.s_distspin->value(), ui.s_bacdspin->value());
}

void QSetParamWnd::setzar()
{
	double temp = ui.z_rotatespin->value();
	double prop_r = temp / 100;
	temp = ui.z_zoomspin->value();
	double prop_z = temp / 100;
	pparam->SetGestureParamZoomARoatate(prop_r, prop_z);
}

void QSetParamWnd::clickdisable(bool b)
{
	eGestureType gtype = GClick;
	pparam->SetDisable(gtype, b);
}

void QSetParamWnd::dclickdisable(bool b)
{
	eGestureType gtype = GDoubleClick;
	pparam->SetDisable(gtype, b);
}

void QSetParamWnd::movetodisable(bool b)
{
	eGestureType gtype = GMoveTo;
	pparam->SetDisable(gtype, b);
}

void QSetParamWnd::lpushdisable(bool b)
{
	eGestureType gtype = GLongPush;
	pparam->SetDisable(gtype, b);
}

void QSetParamWnd::flipdowndisable(bool b)
{
	eGestureType gtype = GFlipDown;
	pparam->SetDisable(gtype, b);
}

void QSetParamWnd::flipleftdisable(bool b)
{

	eGestureType gtype = GFlipLeft;
	pparam->SetDisable(gtype, b);
}

void QSetParamWnd::fliprightdisable(bool b)
{
	eGestureType gtype = GFlipRight;
	pparam->SetDisable(gtype, b);
}

void QSetParamWnd::flipupdisable(bool b)
{
	eGestureType gtype = GFlipUp;
	pparam->SetDisable(gtype, b);
}

void QSetParamWnd::switchhoridisable(bool b)
{
	eGestureType gtype = GSwithHori;
	pparam->SetDisable(gtype, b);
}

void QSetParamWnd::switchvertdisable(bool b)
{
	eGestureType gtype = GSwithVert;
	pparam->SetDisable(gtype, b);
}

void QSetParamWnd::zoomdisable(bool b)
{
	eGestureType gtype = GZoom;
	pparam->SetDisable(gtype, b);
}

void QSetParamWnd::rotatedisable(bool b)
{
	eGestureType gtype = GRotate;
	pparam->SetDisable(gtype, b);
}

void QSetParamWnd::dragdisable(bool b)
{
	eGestureType gtype = GDrag;
	pparam->SetDisable(gtype, b);
}

void QSetParamWnd::saveall()
{
 	setbasic();
	setclick();
	setdclick();
	setflip();
	setslideback();
	setzar();
}
#include"include/QMySlider.h"

QMySlider::QMySlider(QWidget* parent)
	: QSlider(parent)
{
	
}


QMySlider::~QMySlider()
{
	
}

void QMySlider::mousePressEvent(QMouseEvent* ev)
{
	double pos = (double)ev->pos().x() / (double)width();//当前鼠标位置比率
	setValue(pos * this->maximum());//设置位置
	QSlider::mousePressEvent(ev);
}


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
	double pos = (double)ev->pos().x() / (double)width();//��ǰ���λ�ñ���
	setValue(pos * this->maximum());//����λ��
	QSlider::mousePressEvent(ev);
}


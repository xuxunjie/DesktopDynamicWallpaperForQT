#pragma once
#include <QOpenGLWidget>
#include<qpainter.h>
class QVideoWidget : public QOpenGLWidget
{
    Q_OBJECT

public:
    QVideoWidget(QWidget* parent = Q_NULLPTR);
    ~QVideoWidget();
    void paintEvent(QPaintEvent* event) ;
    void timerEvent(QTimerEvent* event);//¶¨Ê±Æ÷
private:
    QImage m_img;
};

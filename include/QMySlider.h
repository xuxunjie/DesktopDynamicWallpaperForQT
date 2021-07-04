#pragma once
#include <QSlider>
#include<QMouseEvent>
class QMySlider : public QSlider
{
    Q_OBJECT

public:
    QMySlider(QWidget* parent = Q_NULLPTR);
    ~QMySlider();
    void mousePressEvent(QMouseEvent* ev);//鼠标按下事件
private:
   
};

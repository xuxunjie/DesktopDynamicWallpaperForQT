#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_QVideoPlayer.h"
#include"include/QPlayListThread.h"
#include"include/QVideoWidget.h"
class QVideoPlayer : public QMainWindow
{
    Q_OBJECT

public:
    QVideoPlayer(QWidget *parent = Q_NULLPTR);
    ~QVideoPlayer();
    void timerEvent(QTimerEvent* event);//��ʱ��
    void closeEvent(QCloseEvent* event);
   
private slots:
    void openVedio();
    void playAndStop();//��������ͣ
    void skip();
    void setTotalTime(int);
    void sliderPressed();//���½�����ʱ		
    void sliderReleased();//�ɿ�������ʱ
    void changeAudioVolumn(double);
private:
    QStringList getVedios(QString dir);
private:
    Ui::QVideoPlayerClass ui;
    QVideoWidget* m_videoWidget;
    QPlayListThread* m_playThread;
    bool m_isPressSlider;
};

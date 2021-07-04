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
    void timerEvent(QTimerEvent* event);//定时器
    void closeEvent(QCloseEvent* event);
   
private slots:
    void openVedio();
    void playAndStop();//播放与暂停
    void skip();
    void setTotalTime(int);
    void sliderPressed();//按下进度条时		
    void sliderReleased();//松开进度条时
    void changeAudioVolumn(double);
private:
    QStringList getVedios(QString dir);
private:
    Ui::QVideoPlayerClass ui;
    QVideoWidget* m_videoWidget;
    QPlayListThread* m_playThread;
    bool m_isPressSlider;
};

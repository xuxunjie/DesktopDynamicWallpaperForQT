#include "include/QVideoPlayer.h"
#include<QFileDialog>
#include<qdesktopwidget.h>
#include<QDir>
#include<QFileInfoList>
#include"include/QXFFmpeg.h"
#include<Windows.h>
#pragma comment(lib,"user32.lib")

using namespace std;
HWND _workerw = nullptr;
HWND windowHandle = nullptr;
int m_result = 0;
inline BOOL CALLBACK EnumWindowsProc(_In_ HWND tophandle, _In_ LPARAM topparamhandle)
{
	HWND defview = FindWindowEx(tophandle, 0, "SHELLDLL_DefView", nullptr);
	if (defview != nullptr)
	{
		_workerw = FindWindowEx(0, tophandle, "WorkerW", 0);
	}
	return true;
}//��������Ļص�����
HWND GetWorkerW() {
	
	windowHandle = FindWindow("Progman", nullptr);
	SendMessageTimeout(windowHandle, 0x052c, 0, 0, SMTO_NORMAL, 0x3e8, (PDWORD_PTR)&m_result);
	EnumWindows(EnumWindowsProc, (LPARAM)nullptr);
	ShowWindow(_workerw,SW_HIDE);
	return windowHandle;
}//��ȡ������ײ�ľ��

QVideoPlayer::QVideoPlayer(QWidget *parent)
    : QMainWindow(parent)
{
	ui.setupUi(this);
	m_playThread = NULL;
	connect(ui.m_openButton, SIGNAL(clicked()), this, SLOT(openVedio()));
	connect(ui.m_skipButton, SIGNAL(clicked()), this, SLOT(skip()));
	connect(ui.m_playAndStopButton, SIGNAL(clicked()), this, SLOT(playAndStop()));
	connect(ui.horizontalSlider, SIGNAL(sliderPressed()), this, SLOT(sliderPressed()));
	connect(ui.horizontalSlider, SIGNAL(sliderReleased()), this, SLOT(sliderReleased()));
	connect(ui.m_audioVolume, SIGNAL(valueChanged(double)), this, SLOT(changeAudioVolumn(double)));
	startTimer(40);//���ö�ʱ��
	m_isPressSlider = false; 
	QDesktopWidget* desktopWidget = QApplication::desktop();
	QRect deskRect = desktopWidget->availableGeometry();
	double availableScreenX = deskRect.width();
	double availableScreenY = deskRect.height();
	
	m_videoWidget = new QVideoWidget();
	m_videoWidget->setGeometry(deskRect);
	m_videoWidget->setWindowFlags(Qt::FramelessWindowHint);
	m_videoWidget->show();
	
	SetParent((HWND)m_videoWidget->winId(), GetWorkerW());
	//����Ƶ������ӵ���������

	QStringList videos = getVedios(QString::fromLocal8Bit("F:\\��ֽ\\��̬"));
	m_playThread = new QPlayListThread(videos);
	connect(m_playThread, SIGNAL(openVedio(int)), this, SLOT(setTotalTime(int)));
	m_playThread->start();
}

void QVideoPlayer::openVedio()
{
	QString path = QFileDialog::getExistingDirectory(this, QString::fromLocal8Bit("ѡ����Ƶ�ļ�·��"), "C:\\Users\\Administrator\\Desktop\\mp4");//����Ƶ�ļ�
	if (path.isEmpty())
		return;
	QStringList videos= getVedios(path);
	if (m_playThread != NULL)
	{
		m_playThread->exitThread();
		m_playThread->quit();
		m_playThread->wait();
		m_playThread->terminate();
		delete m_playThread;
	}
	m_playThread = new QPlayListThread(videos);
	connect(m_playThread, SIGNAL(openVedio(int)), this, SLOT(setTotalTime(int)));
	m_playThread->start();
	
}

QVideoPlayer::~QVideoPlayer()
{
	if (m_playThread != NULL)
	{
		m_playThread->exitThread();
		m_playThread->quit();
		m_playThread->wait();
		m_playThread->terminate();
		delete m_playThread;
	}
	/*if (m_videoWidget)
	{
		delete m_videoWidget;
	}*/
}

void QVideoPlayer::timerEvent(QTimerEvent* event)
{
	int min = (QXFFmpeg::Get()->m_pts) / 60;//��Ƶ���ŵ�ǰ�ķ���	
	int sec = (QXFFmpeg::Get()->m_pts) % 60;//��Ƶ���ŵ�ǰ����
	ui.m_currentTime->setText(QString("%1:%2").arg(min).arg(sec));

	if (QXFFmpeg::Get()->m_totalMs > 0&&!m_isPressSlider)//�ж���Ƶ����ʱ��
	{
		float rate = (float)QXFFmpeg::Get()->m_pts / (float)QXFFmpeg::Get()->m_totalMs;//��ǰ���ŵ�ʱ������Ƶ��ʱ��ı�ֵ
		ui.horizontalSlider->setValue(rate * 1000);//���õ�ǰ������λ��
	}
}

void QVideoPlayer::closeEvent(QCloseEvent* event)
{
	delete m_videoWidget;
	event->accept();
}

void QVideoPlayer::playAndStop()
{
	bool f = !QXFFmpeg::Get()->m_isPlay;
	QXFFmpeg::Get()->m_isPlay = f;
	if (!f)//���������
	{
		ui.m_playAndStopButton->setText(QString::fromLocal8Bit("����"));//��ʾ���Ű�ť״̬
	}
	else
	{
		ui.m_playAndStopButton->setText(QString::fromLocal8Bit("��ͣ"));//��ʾ��ͣ���Ű�ť״̬
	}
}

void QVideoPlayer::skip()
{
	m_playThread->skip();
}

void QVideoPlayer::setTotalTime(int time)
{
	int min = time / 60;
	int sec = time % 60;
	ui.m_totalTime->setText(QString("%1:%2").arg(min).arg(sec));
}

void QVideoPlayer::sliderPressed()
{
	m_isPressSlider = true;
}

void QVideoPlayer::sliderReleased()
{
	m_isPressSlider = false;
	float pos = 0;

	//�ɿ�ʱ��ʱ��������λ���뻬�������ܳ���
	pos = (float)ui.horizontalSlider->value() / (float)(ui.horizontalSlider->maximum() + 1);
	QXFFmpeg::Get()->seek(pos);
}

void QVideoPlayer::changeAudioVolumn(double value)
{
	QXFFmpeg::Get()->m_vedioVolume = value;
}

QStringList QVideoPlayer::getVedios(QString dir)
{
	QStringList videos;
	QDir path(dir);
	path.setFilter(QDir::Files | QDir::Hidden | QDir::NoSymLinks);
	path.setSorting(QDir::Size | QDir::Reversed);

	QFileInfoList list = path.entryInfoList();
	
	for (int i = 0; i < list.size(); ++i) {
		QFileInfo fileInfo = list.at(i);
		videos.append(fileInfo.filePath());
	}
	return videos;
}

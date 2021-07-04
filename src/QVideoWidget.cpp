#include"include/QVideoWidget.h"
#include"include/QXFFmpeg.h"
#include"include/QVideoThread.h"
#include<qapplication.h>

QVideoWidget::QVideoWidget(QWidget* parent)
	: QOpenGLWidget(parent)
{
	//QXFFmpeg::Get()->openVideo(QString::fromLocal8Bit("D:\\VS��Ŀ\\DesktopDynamicWallpaperForQT\\mp4\\1.mp4"));//����Ƶ
	startTimer(10);//���ö�ʱ��
	QVideoThread::Get()->start();//������ȡ��Ƶ�����롢���Ʋ����ٶ��߳�
}


QVideoWidget::~QVideoWidget()
{
	QXFFmpeg::Get()->closeVideo();
	QVideoThread::Get()->exitThread();
	QVideoThread::Get()->quit();
	QVideoThread::Get()->wait();
	QVideoThread::Get()->terminate();
}

void QVideoWidget::paintEvent(QPaintEvent* event)
{
	//����
	static QImage* image = NULL;
	AVFrame* yuv = QXFFmpeg::Get()->getFrame();
	if (yuv == NULL) return;
	if(yuv->width == 0 && yuv->height == 0)return;
	double windowRate = width()*1.0 / height();
	double photoRate = yuv->width * 1.0 / yuv->height;
	
	int realWidth = width();
	int realHeight = height();
	if (yuv->width != 0 && yuv->height != 0)
	{
		if (windowRate >= photoRate)
		{
			realWidth = std::min(yuv->height, height()) * photoRate;
			realHeight = std::min(yuv->height, height());
		}
		else
		{
			realWidth = std::min(yuv->width, width());
			realHeight = std::min(yuv->width, width()) / photoRate;
		}
	}
	if (image == NULL|| image->size() != QSize(realWidth, realHeight))
	{
		uchar* buf = new uchar[realWidth * realHeight * 4];//��Ž�������Ƶ�ռ�
		image = new QImage(buf, realWidth, realHeight, QImage::Format_ARGB32);
	}
	
	
	//����������Ƶ֡ת��ΪRGB
	QXFFmpeg::Get()->toRGB(yuv, (char*)image->bits(), realWidth, realHeight);
	//ͼ�����
	int realX = 0;
	int realY = 0;
	realX = abs(width() - realWidth) / 2;
	realY = abs(height() - realHeight) / 2;
	QPainter painter(this);//hua
	painter.fillRect(0, 0, width(), height(), QColor(255, 255, 255));
	painter.drawImage(QPoint(realX, realY), *image);//����FFMpeg��������Ƶ
}
void QVideoWidget::timerEvent(QTimerEvent* event)
{
	this->update();//��ʱ������
}

#include"include/QVideoWidget.h"
#include"include/QXFFmpeg.h"
#include"include/QVideoThread.h"
#include<qapplication.h>

QVideoWidget::QVideoWidget(QWidget* parent)
	: QOpenGLWidget(parent)
{
	//QXFFmpeg::Get()->openVideo(QString::fromLocal8Bit("D:\\VS项目\\DesktopDynamicWallpaperForQT\\mp4\\1.mp4"));//打开视频
	startTimer(10);//设置定时器
	QVideoThread::Get()->start();//开启读取视频、解码、控制播放速度线程
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
	//绘制
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
		uchar* buf = new uchar[realWidth * realHeight * 4];//存放解码后的视频空间
		image = new QImage(buf, realWidth, realHeight, QImage::Format_ARGB32);
	}
	
	
	//将解码后的视频帧转化为RGB
	QXFFmpeg::Get()->toRGB(yuv, (char*)image->bits(), realWidth, realHeight);
	//图像居中
	int realX = 0;
	int realY = 0;
	realX = abs(width() - realWidth) / 2;
	realY = abs(height() - realHeight) / 2;
	QPainter painter(this);//hua
	painter.fillRect(0, 0, width(), height(), QColor(255, 255, 255));
	painter.drawImage(QPoint(realX, realY), *image);//绘制FFMpeg解码后的视频
}
void QVideoWidget::timerEvent(QTimerEvent* event)
{
	this->update();//定时器更新
}

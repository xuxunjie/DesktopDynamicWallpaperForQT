#include"include/QVideoThread.h"
#include"include/QXFFmpeg.h"
#include"include/QAudioPlay.h"

QVideoThread::QVideoThread()
{	
}


QVideoThread::~QVideoThread()
{
}

void QVideoThread::run()
{
	m_isExit = false;
	char out[10000] = { 0 };
	while (!m_isExit)//线程未退出
	{
		if (!QXFFmpeg::Get()->m_isPlay)//如果为暂停状态，不处理
		{
			msleep(10);
			continue;
		}
		

		int free = QAudioPlay::Get()->getFree();//此时缓冲区的空间大小
		if (free < 10000)
		{
			msleep(1);
			continue;
		}
		AVPacket pkt = QXFFmpeg::Get()->readFrame();
		if (pkt.size <= 0)//未打开视频
		{
			msleep(10);
			continue;
		}

		if (pkt.stream_index == QXFFmpeg::Get()->m_audioStream)
		{
			QXFFmpeg::Get()->decodeFrame(&pkt);//解码音频
			av_packet_unref(&pkt);//释放pkt包
			int len = QXFFmpeg::Get()->toPCM(out);//重采样音频
			QAudioPlay::Get()->write(out, len);//写入音频 
			continue;
		}
		
		QXFFmpeg::Get()->decodeFrame(&pkt);//解码视频帧
		av_packet_unref(&pkt);
		/*if (QXFFmpeg::Get()->m_fps > 0)//控制解码的进度
			msleep(1000 / QXFFmpeg::Get()->m_fps);*/
	}
	m_isExit = false;
}

void QVideoThread::exitThread()
{
	m_isExit = true;
}

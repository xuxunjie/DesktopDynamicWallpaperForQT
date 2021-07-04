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
	while (!m_isExit)//�߳�δ�˳�
	{
		if (!QXFFmpeg::Get()->m_isPlay)//���Ϊ��ͣ״̬��������
		{
			msleep(10);
			continue;
		}
		

		int free = QAudioPlay::Get()->getFree();//��ʱ�������Ŀռ��С
		if (free < 10000)
		{
			msleep(1);
			continue;
		}
		AVPacket pkt = QXFFmpeg::Get()->readFrame();
		if (pkt.size <= 0)//δ����Ƶ
		{
			msleep(10);
			continue;
		}

		if (pkt.stream_index == QXFFmpeg::Get()->m_audioStream)
		{
			QXFFmpeg::Get()->decodeFrame(&pkt);//������Ƶ
			av_packet_unref(&pkt);//�ͷ�pkt��
			int len = QXFFmpeg::Get()->toPCM(out);//�ز�����Ƶ
			QAudioPlay::Get()->write(out, len);//д����Ƶ 
			continue;
		}
		
		QXFFmpeg::Get()->decodeFrame(&pkt);//������Ƶ֡
		av_packet_unref(&pkt);
		/*if (QXFFmpeg::Get()->m_fps > 0)//���ƽ���Ľ���
			msleep(1000 / QXFFmpeg::Get()->m_fps);*/
	}
	m_isExit = false;
}

void QVideoThread::exitThread()
{
	m_isExit = true;
}

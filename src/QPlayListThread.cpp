#include"include/QPlayListThread.h"
#include"include/QXFFmpeg.h"
#include"include/QAudioPlay.h"

QPlayListThread::QPlayListThread(QStringList list)
{	
	m_list = list;
	m_isSkip = false;
}


QPlayListThread::~QPlayListThread()
{
}

void QPlayListThread::run()
{
	m_isSkip = false;
	m_isExit = false;
	for (size_t i = 0; i < m_list.size(); i++)
	{
		if (m_isExit)
			return;
		int time = QXFFmpeg::Get()->openVideo(m_list[i].toUtf8());
		if (time!=0)
		{
			QAudioPlay::Get()->m_sampleRate = QXFFmpeg::Get()->m_sampleRate;
			QAudioPlay::Get()->m_channel = QXFFmpeg::Get()->m_channel;
			QAudioPlay::Get()->m_sampleSize = 16;
			QAudioPlay::Get()->start();//Æô¶¯ÒôÆµ
		
			emit openVedio(time);
			while (true)
			{
				if (m_isExit)
					return;
				if (m_isSkip)
				{
					m_isSkip = false;
					break;
				}
				if (QXFFmpeg::Get()->m_pts >= (float)QXFFmpeg::Get()->m_totalMs)
					break;
				sleep(0.5);
			}
		}
		QXFFmpeg::Get()->closeVideo();
	}
	m_isExit = false;
}

void QPlayListThread::exitThread()
{
	m_isExit = true;
}

void QPlayListThread::skip()
{
	m_isSkip = true;
}

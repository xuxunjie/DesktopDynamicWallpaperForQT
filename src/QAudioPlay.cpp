#include "include/QAudioPlay.h"

QAudioPlay::QAudioPlay()
{
	
}


bool QAudioPlay::start()
{
	stop();
	m_mutex.lock();
	//播放音频
	QAudioFormat fmt;//设置音频输出格式
	fmt.setSampleRate(m_sampleRate);//1秒的音频采样率
	fmt.setSampleSize(m_sampleSize);//声音样本的大小
	fmt.setChannelCount(m_channel);//声道
	fmt.setCodec("audio/pcm");//解码格式
	fmt.setByteOrder(QAudioFormat::LittleEndian);
	fmt.setSampleType(QAudioFormat::UnSignedInt);//设置音频类型
	m_audioOutput = new QAudioOutput(fmt);
	m_audioOutput->setVolume(2);
	m_io = m_audioOutput->start();//播放开始
	m_mutex.unlock();
	return true;
}

void QAudioPlay::play(bool isplay)
{
	m_mutex.lock();
	if (!m_audioOutput)
	{
		m_mutex.unlock();
		return;
	}
	if (isplay)
	{
		m_audioOutput->resume();//恢复播放
	}
	else
	{
		m_audioOutput->suspend();//暂停播放
	}
	m_mutex.unlock();
}

bool QAudioPlay::write(const char* data, int datasize)
{
	m_mutex.lock();
	if (m_io)
		m_io->write(data, datasize);//将获取的音频写入到缓冲区中
	m_mutex.unlock();
	return true;
}

void QAudioPlay::stop()
{
	m_mutex.lock();
	if (m_audioOutput)//为打开Audiom_audioOutput
	{
		m_audioOutput->stop();
		delete m_audioOutput;
		m_audioOutput = NULL;
		m_io = NULL;
	}
	m_mutex.unlock();
}

int QAudioPlay::getFree()
{
	m_mutex.lock();
	if (!m_audioOutput)
	{
		m_mutex.unlock();
		return 0;
	}
	int free = m_audioOutput->bytesFree();//剩余的空间

	m_mutex.unlock();

	return free;
}

QAudioPlay::~QAudioPlay()
{
	stop();
}

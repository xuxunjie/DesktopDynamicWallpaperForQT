#include "include/QAudioPlay.h"

QAudioPlay::QAudioPlay()
{
	
}


bool QAudioPlay::start()
{
	stop();
	m_mutex.lock();
	//������Ƶ
	QAudioFormat fmt;//������Ƶ�����ʽ
	fmt.setSampleRate(m_sampleRate);//1�����Ƶ������
	fmt.setSampleSize(m_sampleSize);//���������Ĵ�С
	fmt.setChannelCount(m_channel);//����
	fmt.setCodec("audio/pcm");//�����ʽ
	fmt.setByteOrder(QAudioFormat::LittleEndian);
	fmt.setSampleType(QAudioFormat::UnSignedInt);//������Ƶ����
	m_audioOutput = new QAudioOutput(fmt);
	m_audioOutput->setVolume(2);
	m_io = m_audioOutput->start();//���ſ�ʼ
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
		m_audioOutput->resume();//�ָ�����
	}
	else
	{
		m_audioOutput->suspend();//��ͣ����
	}
	m_mutex.unlock();
}

bool QAudioPlay::write(const char* data, int datasize)
{
	m_mutex.lock();
	if (m_io)
		m_io->write(data, datasize);//����ȡ����Ƶд�뵽��������
	m_mutex.unlock();
	return true;
}

void QAudioPlay::stop()
{
	m_mutex.lock();
	if (m_audioOutput)//Ϊ��Audiom_audioOutput
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
	int free = m_audioOutput->bytesFree();//ʣ��Ŀռ�

	m_mutex.unlock();

	return free;
}

QAudioPlay::~QAudioPlay()
{
	stop();
}

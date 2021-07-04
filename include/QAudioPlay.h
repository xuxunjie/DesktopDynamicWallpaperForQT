#pragma once
#include <QAudioOutput>
#include<QAudioFormat>
#include<QMutex>
class QAudioPlay 
{
	
public:
	static QAudioPlay* Get()//����ģʽ
	{
		static QAudioPlay ff;
		return &ff;
	}

	bool start();//����
	void  play(bool isplay) ;//��ͣ
	bool write(const char* data, int datasize);//����Ƶд��
	void stop();//ֹͣ
	int getFree();//��ȡʣ��ռ�
	~QAudioPlay();
	int m_sampleRate = 48000;//������
	int m_sampleSize = 16;//������С
	int m_channel = 2;///ͨ����

protected:
	QAudioPlay();
private:
	QAudioOutput* m_audioOutput = NULL;
	QIODevice* m_io = NULL;
	QMutex m_mutex;
};

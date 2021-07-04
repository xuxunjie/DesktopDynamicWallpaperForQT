#pragma once
#include <QAudioOutput>
#include<QAudioFormat>
#include<QMutex>
class QAudioPlay 
{
	
public:
	static QAudioPlay* Get()//单例模式
	{
		static QAudioPlay ff;
		return &ff;
	}

	bool start();//启动
	void  play(bool isplay) ;//暂停
	bool write(const char* data, int datasize);//将音频写入
	void stop();//停止
	int getFree();//获取剩余空间
	~QAudioPlay();
	int m_sampleRate = 48000;//样本率
	int m_sampleSize = 16;//样本大小
	int m_channel = 2;///通道数

protected:
	QAudioPlay();
private:
	QAudioOutput* m_audioOutput = NULL;
	QIODevice* m_io = NULL;
	QMutex m_mutex;
};

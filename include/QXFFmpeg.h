#pragma once

#include <QMutex>
#include<QString>
#include<QObject>
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
#include<libswresample/swresample.h>
}

class QXFFmpeg :public QObject
{
	Q_OBJECT
public:
	static QXFFmpeg* Get()//����ģʽ
	{
		static QXFFmpeg ff;
		return &ff;
	}

	///����Ƶ�ļ�������ϴ��Ѿ��򿪻��ȹر�
	///@para path  ��Ƶ�ļ�·��
	///@return int ��ʱ��ʧ�ܴ�����Ϣͨ�� GetError��ȡ
	int openVideo(QString path);//����Ƶ�ļ�
	void closeVideo();//�ر��ļ�
	AVPacket  readFrame();//��ȡ��Ƶ��ÿһ֡������ÿ֡����Ҫ����ռ�
	//��ȡ��ÿ֡���ݺ���Ҫ������н��� ������ʾʱ�䡣
	int decodeFrame(const AVPacket* pkt);
	
	bool seek(float pos);//������Ƶ

	AVFrame* getFrame();
	AVFrame* getAudio();
	
	QString getErrorInfo();//��ȡ������Ϣ
	virtual ~QXFFmpeg();
	//��������YUV��Ƶ֡ת��ΪRGB��ʽ
	bool toRGB(const AVFrame* yuv, char* out, int outwidth, int outheight);
	int toPCM(char* out);
	/*@Func   : SetVolume
		* @brief : ��������
		* @author : linghuzhangmen
		* @[in]   : buf Ϊ��Ҫ������������Ƶ���ݿ��׵�ַָ��
		* @[in]   : sizeΪ����
		* @[in]   : uRepeatΪ�ظ�������ͨ����Ϊ1
		* @[in]   : volΪ���汶��, ����С��1
		*/

	int m_pts = 0;//��ǰʱ��
	int m_totalMs = 0;//��ʱ��
	int m_videoStream = 0;//��Ƶ��
	double m_fps=0;//��Ƶ֡��
	bool m_isPlay = true;//�Ƿ񲥷�
	int m_audioStream=0;//��Ƶ��

	int m_sampleRate = 48000;//������
	int m_sampleSize = 16;//������С
	int m_channel = 2;///ͨ����
	double m_vedioVolume = 1;
signals:
	void log(QString log, int level);
private:
	static double r2d(AVRational r)
	{
		return r.den == 0 ? 0 : (double)r.num / (double)r.den;
	}
	
	void setVolume(char* buf, uint32_t size, uint32_t uRepeat, double vol);
private:
	char errorbuff[1024];//��ʱ�����Ĵ�����Ϣ
	QXFFmpeg();
	QMutex m_mutex;//������������߳�ʱ����ͬʱ��Ķ�д

	AVFormatContext* m_formatCtx = NULL;//���װ������
	AVFrame* m_frameData = NULL;//��������Ƶ֡����
	AVFrame* m_audioData = NULL;//��������֡����
	SwsContext* m_swsCtx = NULL;//��Ƶת����������
	SwrContext* m_swrCtx = NULL;//��֡ת����������
};

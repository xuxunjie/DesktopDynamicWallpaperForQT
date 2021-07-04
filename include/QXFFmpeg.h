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
	static QXFFmpeg* Get()//单件模式
	{
		static QXFFmpeg ff;
		return &ff;
	}

	///打开视频文件，如果上次已经打开会先关闭
	///@para path  视频文件路径
	///@return int 耗时，失败错误信息通过 GetError获取
	int openVideo(QString path);//打开视频文件
	void closeVideo();//关闭文件
	AVPacket  readFrame();//读取视频的每一帧，返回每帧后需要清理空间
	//读取到每帧数据后需要对其进行解码 返回显示时间。
	int decodeFrame(const AVPacket* pkt);
	
	bool seek(float pos);//更新视频

	AVFrame* getFrame();
	AVFrame* getAudio();
	
	QString getErrorInfo();//获取错误信息
	virtual ~QXFFmpeg();
	//将解码后的YUV视频帧转化为RGB格式
	bool toRGB(const AVFrame* yuv, char* out, int outwidth, int outheight);
	int toPCM(char* out);
	/*@Func   : SetVolume
		* @brief : 音量调节
		* @author : linghuzhangmen
		* @[in]   : buf 为需要调节音量的音频数据块首地址指针
		* @[in]   : size为长度
		* @[in]   : uRepeat为重复次数，通常设为1
		* @[in]   : vol为增益倍数, 可以小于1
		*/

	int m_pts = 0;//当前时间
	int m_totalMs = 0;//总时长
	int m_videoStream = 0;//视频流
	double m_fps=0;//视频帧率
	bool m_isPlay = true;//是否播放
	int m_audioStream=0;//音频流

	int m_sampleRate = 48000;//样本率
	int m_sampleSize = 16;//样本大小
	int m_channel = 2;///通道数
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
	char errorbuff[1024];//打开时发生的错误信息
	QXFFmpeg();
	QMutex m_mutex;//互斥变量，多线程时避免同时间的读写

	AVFormatContext* m_formatCtx = NULL;//解封装上下文
	AVFrame* m_frameData = NULL;//解码后的视频帧数据
	AVFrame* m_audioData = NULL;//解码后的音帧数据
	SwsContext* m_swsCtx = NULL;//视频转码器上下文
	SwrContext* m_swrCtx = NULL;//音帧转码器上下文
};

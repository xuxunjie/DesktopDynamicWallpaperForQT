#include "include/QXFFmpeg.h"
#include <windows.h>

QXFFmpeg::QXFFmpeg()
{
	errorbuff[0] = '\0';//初始化
	av_register_all();//注册FFMpeg的库
}


QXFFmpeg::~QXFFmpeg()
{
	closeVideo();
}

int QXFFmpeg::openVideo(QString path)
{
	closeVideo();//打开前先关闭清理
	m_mutex.lock();//锁

	int re = avformat_open_input(&m_formatCtx, path.toStdString().c_str(), 0, 0);//打开解封装器
	if (re != 0)//打开错误时
	{
		m_mutex.unlock();//解锁
		av_strerror(re, errorbuff, sizeof(errorbuff));//错误信息
		emit log(QString("open %1 failed :%2").arg(path).arg(errorbuff), 2);
		return 0;
	}
	//解码器
	for (int i = 0; i < m_formatCtx->nb_streams; i++)
	{
		AVCodecContext* enc = m_formatCtx->streams[i]->codec;//解码上下文

		if (enc->codec_type == AVMEDIA_TYPE_VIDEO)//判断是否为视频
		{
			m_videoStream = i;
			m_fps = r2d(m_formatCtx->streams[i]->avg_frame_rate);
			//videoCtx = enc;
			AVCodec* codec = avcodec_find_decoder(enc->codec_id);//查找解码器
			if (!codec)//未找到解码器
			{
				m_mutex.unlock();
				emit log("video code not find",0);
				return 0;
			}
			int err = avcodec_open2(enc, codec, NULL);//打开解码器
			if (err != 0)//未打开解码器
			{
				m_mutex.unlock();
				char buf[1024] = { 0 };
				av_strerror(err, buf, sizeof(buf));
				printf(buf);
				return 0;
			}
			emit log("open codec success!",0);
		}
		else if (enc->codec_type == AVMEDIA_TYPE_AUDIO)//若为音频流
		{
			m_audioStream = i;//音频流
			AVCodec* codec = avcodec_find_decoder(enc->codec_id);//查找解码器
			if (avcodec_open2(enc, codec, NULL) < 0)
			{
				m_mutex.unlock();
				return 0;
			}
			
			m_sampleRate = enc->sample_rate;//样本率
			m_channel = enc->channels;//通道数
			switch (enc->sample_fmt)//样本大小
			{
			case AV_SAMPLE_FMT_S16://signed 16 bits
				m_sampleSize = 16;
				break;
			case  AV_SAMPLE_FMT_S32://signed 32 bits
				m_sampleSize = 32;
			default:
				break;
			}
			emit log(QString("audio sample rate:%1 sample size:%2 chanle:%3").arg(m_sampleRate).arg(m_sampleSize).arg(m_channel),0);
		}
	}
	m_pts = 0;
	m_totalMs = m_formatCtx->duration / (AV_TIME_BASE);//获取视频的总时间
	emit log(QString("file totalSec is %1-%2").arg(m_totalMs / 60).arg(m_totalMs % 60), 0);//以分秒计时
	m_mutex.unlock();
	return m_totalMs;
}

void QXFFmpeg::closeVideo()
{
	m_mutex.lock();//需要上锁，以防多线程中你这里在close，另一个线程中在读取，
	if (m_formatCtx) avformat_close_input(&m_formatCtx);//关闭ic上下文
	if (m_frameData) av_frame_free(&m_frameData);//关闭时释放解码后的视频帧空间
	if (m_audioData) av_frame_free(&m_audioData);//关闭时释放解码后的音帧空间
	if (m_swsCtx)
	{
		sws_freeContext(m_swsCtx);//释放转码器上下文空间
		m_swsCtx = NULL;
	}
	if (m_swrCtx)
	{
		swr_free(&m_swrCtx);//释放转码器上下文空间
		m_swrCtx = NULL;
	}
	m_mutex.unlock();

}

QString QXFFmpeg::getErrorInfo()
{
	m_mutex.lock();
	std::string re = this->errorbuff;
	QString error = QString::fromStdString(re);
	m_mutex.unlock();
	return error;
}

AVPacket QXFFmpeg::readFrame()
{
	AVPacket pkt;
	memset(&pkt, 0, sizeof(AVPacket));
	m_mutex.lock();
	if (!m_formatCtx)
	{
		m_mutex.unlock();
		return pkt;
	}
	int err = av_read_frame(m_formatCtx, &pkt);//读取视频帧
	if (err != 0)//读取失败
	{
		av_strerror(err, errorbuff, sizeof(errorbuff));
	}
	m_mutex.unlock();
	return pkt;
}

int QXFFmpeg::decodeFrame(const AVPacket* pkt)
{
	m_mutex.lock();
	if (!m_formatCtx)//若未打开视频
	{
		m_mutex.unlock();
		return NULL;
	}
	if (m_frameData == NULL)//申请解码的对象空间
	{
		m_frameData = av_frame_alloc();
	}
	if (m_audioData == NULL)//申请解码的对象空间
	{
		m_audioData = av_frame_alloc();
	}
	AVFrame* frame = m_frameData;//此时的frame是解码后的视频流
	if (pkt->stream_index == m_audioStream)//若未音频
	{
		frame = m_audioData;//此时frame是解码后的音频流
	}

	int re = avcodec_send_packet(m_formatCtx->streams[pkt->stream_index]->codec, pkt);//发送之前读取的视频帧pkt
	if (re != 0)
	{
		m_mutex.unlock();
		return NULL;
	}
	re = avcodec_receive_frame(m_formatCtx->streams[pkt->stream_index]->codec, frame);//解码pkt后存入m_frameData中
	if (re != 0)
	{
		m_mutex.unlock();
		return NULL;
	}
	int pts = frame->pts * r2d(m_formatCtx->streams[pkt->stream_index]->time_base);//当前解码的显示时间
	if (pkt->stream_index == m_audioStream)//为音频流时设置pts
		m_pts = pts;
	m_mutex.unlock();
	return pts;
}

bool QXFFmpeg::toRGB(const AVFrame* yuv, char* out, int outwidth, int outheight)
{
	m_mutex.lock();
	if (!m_formatCtx)//未打开视频文件
	{
		m_mutex.unlock();
		return false;
	}
	AVCodecContext* videoCtx = m_formatCtx->streams[this->m_videoStream]->codec;
	m_swsCtx = sws_getCachedContext(m_swsCtx, videoCtx->width,//初始化一个SwsContext
		videoCtx->height,
		videoCtx->pix_fmt, //输入像素格式
		outwidth, outheight,
		AV_PIX_FMT_BGRA,//输出像素格式
		SWS_BICUBIC,//转码的算法
		NULL, NULL, NULL);

	if (!m_swsCtx)
	{
		m_mutex.unlock();
		emit log("sws_getCachedContext  failed!",2);
		return false;
	}
	uint8_t* data[AV_NUM_DATA_POINTERS] = { 0 };
	data[0] = (uint8_t*)out;//第一位输出RGB
	int linesize[AV_NUM_DATA_POINTERS] = { 0 };

	linesize[0] = outwidth * 4;//一行的宽度，32位4个字节
	int h = sws_scale(m_swsCtx, yuv->data, //当前处理区域的每个通道数据指针
		yuv->linesize,//每个通道行字节数
		0, videoCtx->height,//原视频帧的高度
		data,//输出的每个通道数据指针	
		linesize//每个通道行字节数

	);//开始转码

	if (h > 0)
	{
		emit log(QString("(%1)").arg(h),0);
	}
	m_mutex.unlock();
	return true;
}

AVFrame* QXFFmpeg::getFrame()
{
	return m_frameData;
}

bool QXFFmpeg::seek(float pos)
{
	m_mutex.lock();
	if (!m_formatCtx)//未打开视频
	{
		m_mutex.unlock();
		return false;
	}
	int64_t stamp = 0;
	stamp = pos * m_formatCtx->streams[m_videoStream]->duration;//当前它实际的位置
	m_pts = stamp * r2d(m_formatCtx->streams[m_videoStream]->time_base);//获得滑动条滑动后的时间戳
	int re = av_seek_frame(m_formatCtx, m_videoStream, stamp,
		AVSEEK_FLAG_BACKWARD | AVSEEK_FLAG_FRAME);//将视频移至到当前点击滑动条位置
	avcodec_flush_buffers(m_formatCtx->streams[m_videoStream]->codec);//刷新缓冲,清理掉
	m_mutex.unlock();
	if (re > 0)
		return true;
	return false;
}

AVFrame* QXFFmpeg::getAudio()
{
	return m_audioData;
}

int QXFFmpeg::toPCM(char* out)
{
	m_mutex.lock();
	if (!m_formatCtx || !m_audioData || !out)//文件未打开，解码器未打开，无数据
	{
		m_mutex.unlock();
		return 0;
	}
	AVCodecContext* ctx = m_formatCtx->streams[m_audioStream]->codec;//音频解码器上下文
	if (m_swrCtx == NULL)
	{
		m_swrCtx = swr_alloc();//初始化
		swr_alloc_set_opts(m_swrCtx, ctx->channel_layout,
			AV_SAMPLE_FMT_S16,
			ctx->sample_rate,
			ctx->channels,
			ctx->sample_fmt,
			ctx->sample_rate,
			0, 0
		);
		swr_init(m_swrCtx);
	}
	uint8_t* data[1];
	data[0] = (uint8_t*)out;

	//音频的重采样过程
	int len = swr_convert(m_swrCtx, data, 10000,
		(const uint8_t**)m_audioData->data,
		m_audioData->nb_samples
	);
	if (len <= 0)
	{
		m_mutex.unlock();
		return 0;
	}
	int outsize = av_samples_get_buffer_size(NULL, ctx->channels,
		m_audioData->nb_samples,
		AV_SAMPLE_FMT_S16,
		0);
	//音量调节
	setVolume(out, outsize, 1, m_vedioVolume);
	m_mutex.unlock();
	return outsize;
}

void QXFFmpeg::setVolume(char* buf, uint32_t size, uint32_t uRepeat, double vol)
{
	if (!size)
	{
		return;
	}
	for (int i = 0; i < size; i += 2)
	{
		short wData;
		wData = MAKEWORD(buf[i], buf[i + 1]);
		long dwData = wData;
		for (int j = 0; j < uRepeat; j++)
		{
			dwData = dwData * vol;
			if (dwData < -0x8000)
			{
				dwData = -0x8000;
			}
			else if (dwData > 0x7FFF)
			{
				dwData = 0x7FFF;
			}
		}
		wData = LOWORD(dwData);
		buf[i] = LOBYTE(wData);
		buf[i + 1] = HIBYTE(wData);
	}
}
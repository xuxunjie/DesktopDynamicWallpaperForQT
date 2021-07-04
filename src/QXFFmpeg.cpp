#include "include/QXFFmpeg.h"
#include <windows.h>

QXFFmpeg::QXFFmpeg()
{
	errorbuff[0] = '\0';//��ʼ��
	av_register_all();//ע��FFMpeg�Ŀ�
}


QXFFmpeg::~QXFFmpeg()
{
	closeVideo();
}

int QXFFmpeg::openVideo(QString path)
{
	closeVideo();//��ǰ�ȹر�����
	m_mutex.lock();//��

	int re = avformat_open_input(&m_formatCtx, path.toStdString().c_str(), 0, 0);//�򿪽��װ��
	if (re != 0)//�򿪴���ʱ
	{
		m_mutex.unlock();//����
		av_strerror(re, errorbuff, sizeof(errorbuff));//������Ϣ
		emit log(QString("open %1 failed :%2").arg(path).arg(errorbuff), 2);
		return 0;
	}
	//������
	for (int i = 0; i < m_formatCtx->nb_streams; i++)
	{
		AVCodecContext* enc = m_formatCtx->streams[i]->codec;//����������

		if (enc->codec_type == AVMEDIA_TYPE_VIDEO)//�ж��Ƿ�Ϊ��Ƶ
		{
			m_videoStream = i;
			m_fps = r2d(m_formatCtx->streams[i]->avg_frame_rate);
			//videoCtx = enc;
			AVCodec* codec = avcodec_find_decoder(enc->codec_id);//���ҽ�����
			if (!codec)//δ�ҵ�������
			{
				m_mutex.unlock();
				emit log("video code not find",0);
				return 0;
			}
			int err = avcodec_open2(enc, codec, NULL);//�򿪽�����
			if (err != 0)//δ�򿪽�����
			{
				m_mutex.unlock();
				char buf[1024] = { 0 };
				av_strerror(err, buf, sizeof(buf));
				printf(buf);
				return 0;
			}
			emit log("open codec success!",0);
		}
		else if (enc->codec_type == AVMEDIA_TYPE_AUDIO)//��Ϊ��Ƶ��
		{
			m_audioStream = i;//��Ƶ��
			AVCodec* codec = avcodec_find_decoder(enc->codec_id);//���ҽ�����
			if (avcodec_open2(enc, codec, NULL) < 0)
			{
				m_mutex.unlock();
				return 0;
			}
			
			m_sampleRate = enc->sample_rate;//������
			m_channel = enc->channels;//ͨ����
			switch (enc->sample_fmt)//������С
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
	m_totalMs = m_formatCtx->duration / (AV_TIME_BASE);//��ȡ��Ƶ����ʱ��
	emit log(QString("file totalSec is %1-%2").arg(m_totalMs / 60).arg(m_totalMs % 60), 0);//�Է����ʱ
	m_mutex.unlock();
	return m_totalMs;
}

void QXFFmpeg::closeVideo()
{
	m_mutex.lock();//��Ҫ�������Է����߳�����������close����һ���߳����ڶ�ȡ��
	if (m_formatCtx) avformat_close_input(&m_formatCtx);//�ر�ic������
	if (m_frameData) av_frame_free(&m_frameData);//�ر�ʱ�ͷŽ�������Ƶ֡�ռ�
	if (m_audioData) av_frame_free(&m_audioData);//�ر�ʱ�ͷŽ�������֡�ռ�
	if (m_swsCtx)
	{
		sws_freeContext(m_swsCtx);//�ͷ�ת���������Ŀռ�
		m_swsCtx = NULL;
	}
	if (m_swrCtx)
	{
		swr_free(&m_swrCtx);//�ͷ�ת���������Ŀռ�
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
	int err = av_read_frame(m_formatCtx, &pkt);//��ȡ��Ƶ֡
	if (err != 0)//��ȡʧ��
	{
		av_strerror(err, errorbuff, sizeof(errorbuff));
	}
	m_mutex.unlock();
	return pkt;
}

int QXFFmpeg::decodeFrame(const AVPacket* pkt)
{
	m_mutex.lock();
	if (!m_formatCtx)//��δ����Ƶ
	{
		m_mutex.unlock();
		return NULL;
	}
	if (m_frameData == NULL)//�������Ķ���ռ�
	{
		m_frameData = av_frame_alloc();
	}
	if (m_audioData == NULL)//�������Ķ���ռ�
	{
		m_audioData = av_frame_alloc();
	}
	AVFrame* frame = m_frameData;//��ʱ��frame�ǽ�������Ƶ��
	if (pkt->stream_index == m_audioStream)//��δ��Ƶ
	{
		frame = m_audioData;//��ʱframe�ǽ�������Ƶ��
	}

	int re = avcodec_send_packet(m_formatCtx->streams[pkt->stream_index]->codec, pkt);//����֮ǰ��ȡ����Ƶ֡pkt
	if (re != 0)
	{
		m_mutex.unlock();
		return NULL;
	}
	re = avcodec_receive_frame(m_formatCtx->streams[pkt->stream_index]->codec, frame);//����pkt�����m_frameData��
	if (re != 0)
	{
		m_mutex.unlock();
		return NULL;
	}
	int pts = frame->pts * r2d(m_formatCtx->streams[pkt->stream_index]->time_base);//��ǰ�������ʾʱ��
	if (pkt->stream_index == m_audioStream)//Ϊ��Ƶ��ʱ����pts
		m_pts = pts;
	m_mutex.unlock();
	return pts;
}

bool QXFFmpeg::toRGB(const AVFrame* yuv, char* out, int outwidth, int outheight)
{
	m_mutex.lock();
	if (!m_formatCtx)//δ����Ƶ�ļ�
	{
		m_mutex.unlock();
		return false;
	}
	AVCodecContext* videoCtx = m_formatCtx->streams[this->m_videoStream]->codec;
	m_swsCtx = sws_getCachedContext(m_swsCtx, videoCtx->width,//��ʼ��һ��SwsContext
		videoCtx->height,
		videoCtx->pix_fmt, //�������ظ�ʽ
		outwidth, outheight,
		AV_PIX_FMT_BGRA,//������ظ�ʽ
		SWS_BICUBIC,//ת����㷨
		NULL, NULL, NULL);

	if (!m_swsCtx)
	{
		m_mutex.unlock();
		emit log("sws_getCachedContext  failed!",2);
		return false;
	}
	uint8_t* data[AV_NUM_DATA_POINTERS] = { 0 };
	data[0] = (uint8_t*)out;//��һλ���RGB
	int linesize[AV_NUM_DATA_POINTERS] = { 0 };

	linesize[0] = outwidth * 4;//һ�еĿ�ȣ�32λ4���ֽ�
	int h = sws_scale(m_swsCtx, yuv->data, //��ǰ���������ÿ��ͨ������ָ��
		yuv->linesize,//ÿ��ͨ�����ֽ���
		0, videoCtx->height,//ԭ��Ƶ֡�ĸ߶�
		data,//�����ÿ��ͨ������ָ��	
		linesize//ÿ��ͨ�����ֽ���

	);//��ʼת��

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
	if (!m_formatCtx)//δ����Ƶ
	{
		m_mutex.unlock();
		return false;
	}
	int64_t stamp = 0;
	stamp = pos * m_formatCtx->streams[m_videoStream]->duration;//��ǰ��ʵ�ʵ�λ��
	m_pts = stamp * r2d(m_formatCtx->streams[m_videoStream]->time_base);//��û������������ʱ���
	int re = av_seek_frame(m_formatCtx, m_videoStream, stamp,
		AVSEEK_FLAG_BACKWARD | AVSEEK_FLAG_FRAME);//����Ƶ��������ǰ���������λ��
	avcodec_flush_buffers(m_formatCtx->streams[m_videoStream]->codec);//ˢ�»���,�����
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
	if (!m_formatCtx || !m_audioData || !out)//�ļ�δ�򿪣�������δ�򿪣�������
	{
		m_mutex.unlock();
		return 0;
	}
	AVCodecContext* ctx = m_formatCtx->streams[m_audioStream]->codec;//��Ƶ������������
	if (m_swrCtx == NULL)
	{
		m_swrCtx = swr_alloc();//��ʼ��
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

	//��Ƶ���ز�������
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
	//��������
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
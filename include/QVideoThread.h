#pragma once
#include <QThread>

class QVideoThread : public QThread
{
public:
	static QVideoThread* Get()//��������ģʽ
	{
		static QVideoThread vt;
		return &vt;
	}
	void run();//�̵߳�����
	void exitThread();//�˳��߳�
	QVideoThread();
	virtual ~QVideoThread();
private:
	bool m_isExit;
	
};
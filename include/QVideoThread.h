#pragma once
#include <QThread>

class QVideoThread : public QThread
{
public:
	static QVideoThread* Get()//创建单例模式
	{
		static QVideoThread vt;
		return &vt;
	}
	void run();//线程的运行
	void exitThread();//退出线程
	QVideoThread();
	virtual ~QVideoThread();
private:
	bool m_isExit;
	
};
#pragma once
#include <QThread>
#include<qstringlist.h>
class QPlayListThread : public QThread
{
	Q_OBJECT
public:
	QPlayListThread(QStringList list);
	virtual ~QPlayListThread();
	void run();//线程的运行
	void exitThread();//退出线程
	void skip();//跳过
signals:
	void openVedio(int time);
private:
	bool m_isExit;
	bool m_isSkip;//是否跳到下一个
	QStringList m_list;
};
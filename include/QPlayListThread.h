#pragma once
#include <QThread>
#include<qstringlist.h>
class QPlayListThread : public QThread
{
	Q_OBJECT
public:
	QPlayListThread(QStringList list);
	virtual ~QPlayListThread();
	void run();//�̵߳�����
	void exitThread();//�˳��߳�
	void skip();//����
signals:
	void openVedio(int time);
private:
	bool m_isExit;
	bool m_isSkip;//�Ƿ�������һ��
	QStringList m_list;
};
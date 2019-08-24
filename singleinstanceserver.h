#ifndef SINGLEINSTANCESERVER_H
#define SINGLEINSTANCESERVER_H
#include <QObject>
#include <QString>
#include <QLocalServer>

class SingleInstanceServer : public QObject
{
	Q_OBJECT
private:
	QLocalServer server;
private slots:
	void handleNewConnection();
public:
	SingleInstanceServer();
	bool listen(QString socketPath);
signals:
	void receivedMaximizationRequest();
};

#endif // SINGLEINSTANCESERVER_H

#include <QDataStream>
#include <QLocalSocket>
#include <QFile>

#include "singleinstanceserver.h"

SingleInstanceServer::SingleInstanceServer()
{
	connect(&this->server, &QLocalServer::newConnection, this, &SingleInstanceServer::handleNewConnection);
}

bool SingleInstanceServer::listen(QString socketPath)
{
	QFile::remove(socketPath);
	return this->server.listen(socketPath);
}
void SingleInstanceServer::handleNewConnection()
{
	QScopedPointer<QLocalSocket> socket { this->server.nextPendingConnection() };
	if(!socket.isNull())
	{
		socket->waitForReadyRead();
		QDataStream stream ( socket.get());
		int command;
		stream >> command;
		if(command == 0x01)
		{
			emit receivedMaximizationRequest();
		}

	}
}


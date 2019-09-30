/*
 * Copyright (c) 2018-2019 Albert S. <mail at quitesimple dot org>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */
#include <QApplication>
#include <QFuture>
#include <QFutureWatcher>
#include <QtConcurrent/QtConcurrentRun>
#include <QSettings>
#include <QLocalSocket>
#include "configprovider.h"
#include "window.h"
#include "singleinstanceserver.h"




int main(int argc, char *argv[])
{
	QApplication app(argc, argv);
	QString configDirectoryPath;
	QDir dir;
	if(argc >= 2)
	{
		configDirectoryPath = QCoreApplication::arguments().at(1);
		if(!dir.exists(configDirectoryPath))
		{
			QMessageBox::warning(nullptr, "Directory not found", configDirectoryPath + " was not found");
			return 1;
		}
	}
	else
	{
		configDirectoryPath = QDir::homePath() + "/.config/qsrun/";
	}
	qRegisterMetaType<QVector<QString> >("QVector<QString>");


	if(!dir.exists(configDirectoryPath))
	{
		if(!dir.mkdir(configDirectoryPath))
		{
			QMessageBox::warning(nullptr, "Failed to create dir", configDirectoryPath + " was not found and could not be created!");
			return 1;
		}
	}

	QSettings settings(configDirectoryPath + "qsrun.config", QSettings::NativeFormat);

	ConfigProvider configProvider(configDirectoryPath, settings);
	//TODO if setting single instance mode
	QLocalSocket localSocket;
	localSocket.connectToServer("/tmp/qsrun.socket");
	SingleInstanceServer server;
	if(localSocket.isOpen() && localSocket.isWritable())
	{
		QDataStream stream(&localSocket);
		stream << (int)0x01; //maximize
		localSocket.flush();
		localSocket.waitForBytesWritten();
		localSocket.disconnectFromServer();
		return 0;
	}
	else
	{
		if(!server.listen("/tmp/qsrun.socket"))
		{
			qDebug() << "Failed to listen on socket!";
		}
		Window *w = new Window { configProvider };
		QObject::connect(&server, &SingleInstanceServer::receivedMaximizationRequest, [&w]{
			if(w != nullptr)
			{
				qInfo() << "maximizing as requested by other instance";
				w->showMaximized();
				w->activateWindow();
				w->raise();
				w->focusInput();
			}
		});
		w->showMaximized();
		w->focusInput();

	}



	return app.exec();
}

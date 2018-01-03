#include <QApplication>
#include <QFuture>
#include <QFutureWatcher>
#include <QtConcurrent/QtConcurrentRun>
#include <QSettings>
#include "window.h"




int main(int argc, char *argv[])
{
	QApplication app(argc, argv);
	QString configDirectoryPath;
	if(argc >= 2)
	{
		configDirectoryPath = QCoreApplication::arguments().at(1);
	}
	else
	{
		configDirectoryPath = QDir::homePath() + "/.config/qsRun/";
	}
	qRegisterMetaType<QVector<QString> >("QVector<QString>");
	
	QDir dir(configDirectoryPath);
	if(!dir.exists(configDirectoryPath))
	{
		QMessageBox::warning(nullptr, "Directory not found", configDirectoryPath + " was not found!");
		return 1;
	}
	QSettings settings(configDirectoryPath + "qsrun.config", QSettings::NativeFormat);
	QString dbpath = settings.value("Search/dbpath").toString();
	QString systemApplicationsPath = settings.value("General/systemApplicationsPath", "/usr/share/applications/").toString();

	QVector<EntryConfig> configs;

	try
	{
		ConfigReader reader(configDirectoryPath);
		configs = reader.readConfig();
	}
	catch(std::exception &e)
	{
		std::cerr << e.what() << std::endl;
	}

	Window w(configs, dbpath);

	/*
	 * TODO: Reconsider the need
	 * QFuture<void> future = QtConcurrent::run([&w] {
			ConfigReader systemConfigReader("/usr/share/applications/");
			QList<EntryConfig> systemconfigs = systemConfigReader.readConfig();
			if(systemconfigs.count() > 0)
			{
				w.setSystemConfig(systemconfigs);
				w.systemConfigReady();
			}
		 });*/


	try
	{
		ConfigReader systemConfigReader(systemApplicationsPath);
		QVector<EntryConfig> systemconfigs = systemConfigReader.readConfig();
		if(systemconfigs.count() > 0)
		{
			w.setSystemConfig(systemconfigs);
		}
	}
	catch(std::exception &e)
	{
		std::cerr << e.what() << std::endl;
	}
	
	

	w.showMaximized();
	return app.exec();
}

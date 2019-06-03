#include "configprovider.h"

ConfigProvider::ConfigProvider(QString configDirPath, QSettings &settings)
{
	this->settings = &settings;
	this->configDirPath = configDirPath;
}

QVector<EntryConfig> ConfigProvider::getUserEntries()
{
	ConfigReader reader({this->configDirPath});
	return reader.readConfig();
}

QVector<EntryConfig>  ConfigProvider::getSystemEntries()
{
	QStringList systemApplicationsPaths = settings->value("sysAppsPaths", "/usr/share/applications/").toStringList();
	ConfigReader systemConfigReader(systemApplicationsPaths);
	return systemConfigReader.readConfig();
}

bool ConfigProvider::singleInstanceMode()
{
	return settings->value("singleInstance", true).toBool();
}

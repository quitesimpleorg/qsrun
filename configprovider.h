#ifndef CONFIGPROVIDER_H
#define CONFIGPROVIDER_H
#include <QSettings>
#include "config.h"

class ConfigProvider
{
private:
	QSettings *settings;
	QString configDirPath;
public:
	ConfigProvider(QString configDirPath, QSettings &settings);
	QVector<EntryConfig> getUserEntries();
	QVector<EntryConfig> getSystemEntries();
	bool singleInstanceMode();
};

#endif // CONFIGPROVIDER_H

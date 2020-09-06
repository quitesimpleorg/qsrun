#include "settingsprovider.h"
#include <QFileInfo>
#include <QDir>

SettingsProvider::SettingsProvider(QSettings &settings)
{
	this->settings = &settings;
}

QStringList SettingsProvider::userEntriesPaths() const
{
	//TODO: make it configurable, but we stick with this for now.
	QFileInfo fi(this->settings->fileName());
	return { fi.absoluteDir().absolutePath() };
}

QStringList SettingsProvider::systemApplicationsEntriesPaths() const
{
	return settings->value("sysAppsPaths", "/usr/share/applications/").toStringList();
}

bool SettingsProvider::singleInstanceMode() const
{
	return settings->value("singleInstance", true).toBool();
}



#include "settingsprovider.h"
#include <QDir>
#include <QFileInfo>

SettingsProvider::SettingsProvider(QSettings &settings)
{
	this->settings = &settings;
}

QStringList SettingsProvider::userEntriesPaths() const
{
	// TODO: make it configurable, but we stick with this for now.
	QFileInfo fi(this->settings->fileName());
	return {fi.absoluteDir().absolutePath()};
}

QStringList SettingsProvider::systemApplicationsEntriesPaths() const
{
	return settings->value("sysAppsPaths", "/usr/share/applications/").toStringList();
}

int SettingsProvider::getMaxCols() const
{
	return settings->value("maxColumns", 3).toInt();
}

bool SettingsProvider::singleInstanceMode() const
{
	return settings->value("singleInstance", true).toBool();
}

QString SettingsProvider::getTerminalCommand() const
{
	return settings->value("terminal", "/usr/bin/x-terminal-emulator -e %c").toString();
}

QString SettingsProvider::socketPath() const
{
	return settings->value("singleInstanceSocket", "/tmp/qsrun").toString();
}

QVector<SpecialCommandConfig> SettingsProvider::specialCommands() const
{
	QVector<SpecialCommandConfig> result;

	SpecialCommandConfig uname;
	uname.command = "uname";
	uname.reqArgCount = 0;
	uname.immediateProcessing = true;
	result.append(uname);


	SpecialCommandConfig date;
	date.command = "date";
	date.reqArgCount = 0;
	date.immediateProcessing = true;
	result.append(date);

	SpecialCommandConfig echo;
	echo.command = "echo";
	echo.reqArgCount = 0;
	echo.immediateProcessing = true;
	result.append(echo);
	return result;
}

#ifndef SETTINGSPROVIDER_H
#define SETTINGSPROVIDER_H

#include <stdexcept>
#include <QSettings>

class SettingsProvider
{
private:
	QSettings *settings;
public:
	SettingsProvider(QSettings &settings);
	virtual QStringList userEntriesPaths() const;
	virtual QStringList systemApplicationsEntriesPaths() const;
	virtual bool singleInstanceMode() const;
};

#endif // SETTINGSPROVIDER_H
